// --------------------------------------------------
// 3 Script control delete flagF
// 
// A bandeira só é deletada se não tiver ninguem no território. "controle do expansion"
// --------------------------------------------------

modded class TerritoryFlag
{
    // mapa estático: chave = posição string (com 2 casas), valor = timestamp em ms
    static ref map<string, int> zhf_PlayerDismantledMap = new map<string, int>;

    override void EEDelete(EntityAI parent)
    {
        super.EEDelete(parent);

        if (!GetGame() || !GetGame().IsServer())
            return;

        vector pos = GetPosition();
        string key = ZHF_PosToKey(pos);

        int now = GetGame().GetTime(); // ms
        int stored;
        bool wasPlayer = false;

        if (zhf_PlayerDismantledMap.Find(key, stored))
        {
            if (now - stored <= 10000) // 10 segundos
            {
                wasPlayer = true;
            }
            zhf_PlayerDismantledMap.Remove(key);
        }

        // -------------------------------
        // Desmontada manualmente
        // -------------------------------
        if (wasPlayer)
        {
            Print("[ZHFBaseKit] EEDelete ignorado — desmontada pelo player (pos: " + key + ").");
            return;
        }

        // -------------------------------
        // Delete por inatividade (DayZ)
        // -------------------------------
        Print("[ZHFBaseKit] EEDelete detectado — base abandonada em " + key);

        // LIMPA BASE EM 150 METROS
        ZHF_CleanupBase(pos, Flag_Base.CleanupRadius);

        // -------------------------------
        // CAPTURA PLAYER E STEAMID ANTES DE REMOVER
        // -------------------------------
        string playerName = "Desconhecido";
        string playerId = "";
        int removeIndex = -1;

        for (int i = 0; i < Flag_Base.FlagsData.Count(); i++)
        {
            ZHF_FlagData fd = Flag_Base.FlagsData.Get(i);
            if (!fd) continue;

            float dx = fd.PosX - pos[0];
            float dy = fd.PosY - pos[1];
            float dz = fd.PosZ - pos[2];

            if (Math.AbsFloat(dx) < 0.1 && Math.AbsFloat(dy) < 0.1 && Math.AbsFloat(dz) < 0.1)
            {
                playerName = fd.Name;
                playerId = fd.SteamID;
                removeIndex = i;
                break;
            }
        }

        // -------------------------------
        // ENVIA LOG DISCORD
        // -------------------------------
        ZHF_SendDiscordFlagAbandoned(pos, playerName, playerId);

        // -------------------------------
        // LIMPAR REFERÊNCIAS DO PLAYER
        // -------------------------------
        if (playerId != "" && removeIndex != -1)
        {
            // Remove do RewardedPlayers (Player_Items.json)
            int idx = Flag_Base.RewardedPlayers.Find(playerId);
            if (idx > -1)
                Flag_Base.RewardedPlayers.Remove(idx);

            // Remove do FlaggedPlayers (Player_Flag.json)
            string flaggedId = playerId + "987";
            idx = Flag_Base.FlaggedPlayers.Find(flaggedId);
            if (idx > -1)
                Flag_Base.FlaggedPlayers.Remove(idx);

            // Remove do FlagsData
            Flag_Base.FlagsData.Remove(removeIndex);

            // Salva todos os arquivos
            JsonFileLoader<array<string>>.JsonSaveFile(Flag_Base.FILE_PATH_PLAYERS, Flag_Base.RewardedPlayers);
            JsonFileLoader<array<string>>.JsonSaveFile(Flag_Base.FILE_PATH_FLAGS, Flag_Base.FlaggedPlayers);
            Flag_Base.SaveFlagsData();

            Print("[ZHFBaseKit] Limpeza completa de referências do player concluída: " + playerId);
        }
    }

    // --------------------------------------------------
    // Limpa base abandonada - Vanilla (objetos + veículos)
    // --------------------------------------------------
    static void ZHF_CleanupBase(vector center, float radius)
    {
        int deleted = 0;

        array<Object> objects = new array<Object>;
        array<CargoBase> cargos = new array<CargoBase>;

        // Pega todos os objetos dentro do raio horizontal
        GetGame().GetObjectsAtPosition3D(center, radius, objects, cargos);

        // Definir altura mínima e máxima da limpeza (ajustável)
        float minHeight = center[1] - 80;  //50 metros abaixo da bandeira
        float maxHeight = center[1] + 80; // 50 metros acima da bandeira

        foreach (Object obj : objects)
        {
            if (!obj) continue;

            // Ignora jogadores, animais, infectados e flags
            if (obj.IsInherited(PlayerBase)) continue;
            if (obj.IsInherited(DayZInfected)) continue;
            if (obj.IsInherited(AnimalBase)) continue;

            // Verifica se o objeto está dentro da faixa de altura
            float objY = obj.GetPosition()[1];
            if (objY < minHeight || objY > maxHeight) continue;

            GetGame().ObjectDelete(obj);
            deleted++;
            Print("[ZHFBaseKit] Objeto deletado: " + obj);
        }

        Print("[ZHFBaseKit] Limpeza de base abandonada FINALIZADO. Total deletado: " + deleted);
    }

    // --------------------------------------------------
    // Converte vetor pos em string (2 casas decimais)
    // --------------------------------------------------
    static string ZHF_PosToKey(vector pos)
    {
        string kx = ZHF_FloatTo2(pos[0]);
        string ky = ZHF_FloatTo2(pos[1]);
        string kz = ZHF_FloatTo2(pos[2]);
        return kx + " " + ky + " " + kz;
    }

    // --------------------------------------------------
    // Formata float com 2 casas decimais
    // --------------------------------------------------
    static string ZHF_FloatTo2(float v)
    {
        string s = v.ToString();
        int dot = s.IndexOf(".");
        if (dot == -1) return s + ".00";

        int dec = s.Length() - dot - 1;
        if (dec == 0) return s + "00";
        if (dec == 1) return s + "0";

        return s;
    }

    // --------------------------------------------------
    // Envio webhook base abandonada (embed formatado) com Player e SteamID
    // --------------------------------------------------
    static void ZHF_SendDiscordFlagAbandoned(vector pos, string playerName, string playerId)
    {
        if (!Flag_Base.DiscordWebhookAbandoned || Flag_Base.DiscordWebhookAbandoned == "")
        {
            Print("[ZHFBaseKit] ERRO: Webhook_Base_Abandonada não configurado");
            return;
        }

        string webhook = Flag_Base.DiscordWebhookAbandoned;

        int y, mo, d, h, m, s;
        GetYearMonthDay(y, mo, d);
        GetHourMinuteSecond(h, m, s);

        string date = d.ToStringLen(2) + "/" + mo.ToStringLen(2) + "/" + y.ToStringLen(4);
        string time = h.ToStringLen(2) + ":" + m.ToStringLen(2) + ":" + s.ToStringLen(2);

        // Coordenadas com labels e mais espaço
        string posStr = "```X: " + pos[0].ToString() + "    Y: " + pos[1].ToString() + "    Z: " + pos[2].ToString() + "```";
		string izurviveUrl = Flag_Base.GetIzurviveLink(pos);
		
        // 🔴 vermelho
        int embedColor = 16711680;

        // Payload JSON com Player e SteamID
        string payload = "";
        payload += "{\"embeds\":[{";
        payload += "\"title\":\"BASE ABANDONADA - TERRITÓRIO DELETADO!\","; 
        payload += "\"color\":" + embedColor.ToString() + ",";
        payload += "\"fields\":[";
        payload += "{\"name\":\"Player\",\"value\":\"" + playerName + "\",\"inline\":true},";
        payload += "{\"name\":\"Steam\",\"value\":\"" + playerId + "\",\"inline\":true},";
        payload += "{\"name\":\"Local\",\"value\":\"" + posStr + "\"},";
		payload += "{\"name\":\"Mapa\",\"value\":\"[Abrir no iZurvive](" + izurviveUrl + ")\",\"inline\":false}";
        payload += "],";
        payload += "\"footer\":{\"text\":\"" + date + " - " + time + "\"}";
        payload += "}]}";

        // Envio para o Discord
        RestContext ctx = GetRestApi().GetRestContext(webhook);
        if (!ctx)
        {
            Print("[ZHFBaseKit] ERRO: RestContext falhou.");
            return;
        }

        ctx.SetHeader("application/json");
        ctx.POST(new RestCallback, "", payload);

        Print("[ZHFBaseKit] Log enviado ao Discord: BASE ABANDONADA - TERRITÓRIO DELETADO! (Player: " + playerName + ", Steam: " + playerId + ")");
    }
}
