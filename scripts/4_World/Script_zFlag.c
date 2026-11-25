//2 Mont, desmont flag

// --------------------------------------------------
// Modificado: Flag_Base sem entrega de itens e sem criação de arquivo
// Integrado com ExpansionActionDismantleFlag
// --------------------------------------------------
modded class Flag_Base extends ItemBase
{
    static string FILE_PATH_FLAG;
    static string DiscordWebhookURL;
	static string DiscordWebhookAbandoned;
	static string MapName;
	static float  CleanupRadius = 1.0;
    static ref array<string> RewardedPlayers = new array<string>();
	static ref array<string> AdminSteamIDs = new array<string>;
    static bool BaseStarterDataInitialized = false;

    // --------------------------------------------------
    // Inicialização dos dados da base
    // --------------------------------------------------
    static void InitBaseStarterData()
    {
        if (!GetGame().IsServer()) 
            return;

        string folder = GetHFProfileFolder();
        FILE_PATH_FLAG = folder + "Player_Flag.json";

        if (FileExist(FILE_PATH_FLAG))
        {
            JsonFileLoader<array<string>>.JsonLoadFile(FILE_PATH_FLAG, RewardedPlayers);
            Print("[ZHFBaseKit] Jogadores carregados: " + RewardedPlayers.Count().ToString());
        }
        else
        {
            Print("[ZHFBaseKit] Arquivo Player_Flag.json não existe. Nenhum jogador carregado.");
        }
    }

    static void SaveBaseStarterData()
    {
        JsonFileLoader<array<string>>.JsonSaveFile(FILE_PATH_FLAG, RewardedPlayers);
    }

    // --------------------------------------------------
    // Pasta HF do perfil
    // --------------------------------------------------
    static string GetHFProfileFolder()
    {
        string baseFolder;

        if (GetGame().IsServer() && FileExist("$serverprofile:"))
            baseFolder = "$serverprofile:\\HF\\";
        else
            baseFolder = "$profile:\\HF\\";

        if (!FileExist(baseFolder))
            MakeDirectory(baseFolder);

        string subFolder = baseFolder + "BaseKit\\";
        if (!FileExist(subFolder))
            MakeDirectory(subFolder);

        return subFolder;
    }

    // --------------------------------------------------
    // Função de exceção ADM
    // --------------------------------------------------
    static bool IsAdminExempt(PlayerBase player)
    {
        if (!player || !player.GetIdentity())
            return false;

        string steamId = player.GetIdentity().GetPlainId();

        if (!AdminSteamIDs)
            return false;

        return (AdminSteamIDs.Find(steamId) > -1);
    }

    // --------------------------------------------------
	// Envia log ao discord ao montar flag (EMBED)
	// --------------------------------------------------
	void SendDiscordWebhookFlag(PlayerBase player, vector pos)
	{
		if (!player || !DiscordWebhookURL || DiscordWebhookURL == "")
			return;

		int y, mo, d;
		int h, m, s;
		GetYearMonthDay(y, mo, d);
		GetHourMinuteSecond(h, m, s);

		string date = d.ToStringLen(2) + "/" + mo.ToStringLen(2) + "/" + y.ToStringLen(4);
		string time = h.ToStringLen(2) + ":" + m.ToStringLen(2) + ":" + s.ToStringLen(2);

		string playerId   = player.GetIdentity().GetPlainId();
		string playerName = player.GetIdentity().GetName();

		// Coordenadas com labels e mais espaço
		string posStr = "```X: " + pos[0].ToString() + "    Y: " + pos[1].ToString() + "    Z: " + pos[2].ToString() + "```";
		string izurviveUrl = GetIzurviveLink(pos);

		// 🟢 verde
		int embedColor = 65280;

		string payload = "";
		payload += "{\"embeds\":[{";
		payload += "\"title\":\"BANDEIRA MONTADA\",";
		payload += "\"color\":" + embedColor.ToString() + ",";
		payload += "\"fields\":[";
		payload += "{\"name\":\"Player\",\"value\":\"" + playerName + "\",\"inline\":true},";
		payload += "{\"name\":\"Steam\",\"value\":\"" + playerId + "\",\"inline\":true},";
		payload += "{\"name\":\"Local\",\"value\":\"" + posStr + "\"},";
		payload += "{\"name\":\"Mapa\",\"value\":\"[Abrir no iZurvive](" + izurviveUrl + ")\",\"inline\":false}";
		payload += "],";
		payload += "\"footer\":{\"text\":\"" + date + " - " + time + "\"}";
		payload += "}]}";

		RestContext ctx = GetRestApi().GetRestContext(DiscordWebhookURL);
		if (!ctx) return;

		ctx.SetHeader("application/json");
		ctx.POST(new RestCallback, "", payload);
	}


    // --------------------------------------------------
	// Envia log ao discord ao desmontar a flag
	// --------------------------------------------------
	static void SendDiscordWebhookFlagDismantle(PlayerBase player, vector pos)
	{
		if (!player || !DiscordWebhookURL || DiscordWebhookURL == "")
			return;

		int y, mo, d;
		int h, m, s;
		GetYearMonthDay(y, mo, d);
		GetHourMinuteSecond(h, m, s);

		string date = d.ToStringLen(2) + "/" + mo.ToStringLen(2) + "/" + y.ToStringLen(4);
		string time = h.ToStringLen(2) + ":" + m.ToStringLen(2) + ":" + s.ToStringLen(2);

		string playerId   = player.GetIdentity().GetPlainId();
		string playerName = player.GetIdentity().GetName();

		// Coordenadas com labels e mais espaço
		string posStr = "```X: " + pos[0].ToString() + "    Y: " + pos[1].ToString() + "    Z: " + pos[2].ToString() + "```";
		string izurviveUrl = GetIzurviveLink(pos);
		
		// 🔴 vermelho
		int embedColor = 16711680;

		string payload = "";
		payload += "{\"embeds\":[{";
		payload += "\"title\":\"BANDEIRA DESMONTADA\",";
		payload += "\"color\":" + embedColor.ToString() + ",";
		payload += "\"fields\":[";
		payload += "{\"name\":\"Player\",\"value\":\"" + playerName + "\",\"inline\":true},";
		payload += "{\"name\":\"Steam\",\"value\":\"" + playerId + "\",\"inline\":true},";
		payload += "{\"name\":\"Local\",\"value\":\"" + posStr + "\"},";
		payload += "{\"name\":\"Mapa\",\"value\":\"[Abrir no iZurvive](" + izurviveUrl + ")\",\"inline\":false}";
		payload += "],";
		payload += "\"footer\":{\"text\":\"" + date + " - " + time + "\"}";
		payload += "}]}";

		RestContext ctx = GetRestApi().GetRestContext(DiscordWebhookURL);
		if (!ctx) return;

		ctx.SetHeader("application/json");
		ctx.POST(new RestCallback, "", payload);
	}
	
	// --------------------------------------------------												   
    // Evento ao anexar a bandeira em um objeto
    // --------------------------------------------------
    override void OnWasAttached(EntityAI parent, int slot_id)
    {
        super.OnWasAttached(parent, slot_id);

        if (!GetGame().IsServer())
            return;

        if (!parent || !parent.IsKindOf("BaseBuildingBase"))
            return;

        if (!BaseStarterDataInitialized)
        {
            InitBaseStarterData();
            BaseStarterDataInitialized = true;
        }

        PlayerBase player = GetNearestPlayer(parent.GetPosition());
        if (!player)
            return;

        // ===============================
        // EXCEÇÃO ADM
        // ===============================
        if (IsAdminExempt(player))
        {
            Print("[ZHFBaseKit] ADM detectado — regras de BaseKit ignoradas.");
            return;
        }

        SendDiscordWebhookFlag(player, parent.GetPosition());

        // Mantido sem alterações: não grava ID aqui
    }

    // --------------------------------------------------
    // Procura jogador mais próximo
    // --------------------------------------------------
    PlayerBase GetNearestPlayer(vector pos)
    {
        array<Man> players = new array<Man>();
        GetGame().GetPlayers(players);

        PlayerBase nearest;
        float minDist = 99999;

        foreach (Man p : players)
        {
            if (!p) continue;
            float dist = vector.Distance(p.GetPosition(), pos);
            if (dist < minDist)
            {
                minDist = dist;
                nearest = PlayerBase.Cast(p);
            }
        }

        if (minDist <= 10)
            return nearest;

        return null;
    }

    // --------------------------------------------------
    // Função para remover jogador da lista quando desmonta bandeira via Expansion
    // --------------------------------------------------
    static void OnExpansionFlagDismantled(PlayerBase player)
	{
		if (!player)
			return;

		if (!BaseStarterDataInitialized)
			InitBaseStarterData();

		string playerId = player.GetIdentity().GetPlainId();
		string flaggedId = playerId + "987"; // ID com sufixo

		int index = FlaggedPlayers.Find(flaggedId); // verifica bandeira
		if (index > -1)
		{
			FlaggedPlayers.Remove(index); // remove da memória
			//SaveBaseStarterData();       // salva Player_Flag.json
			JsonFileLoader<array<string>>.JsonSaveFile(FILE_PATH_FLAGS, FlaggedPlayers); // salva no arquivo
			
			// ✅ NOVO: Envio do log ao Discord
			vector pos = player.GetPosition();
			SendDiscordWebhookFlagDismantle(player, pos);
		
			player.MessageAction("[ZHF] Bandeira desmontada! Agora você pode montar outra.");
		}
	}
	
	// --------------------------------------------------
	// Gera link do iZurvive usando MapName do Config.json
	// --------------------------------------------------
	static string GetIzurviveLink(vector pos)
	{
		// Fallback de segurança
		string mapName = MapName;
		if (!mapName || mapName == string.Empty)
			mapName = "chernarusplus";

		// iZurvive usa X e Z
		string x = Math.Round(pos[0]).ToString();
		string z = Math.Round(pos[2]).ToString();

		return "https://www.izurvive.com/" + mapName + "/#location=" + x + ";" + z;
	}

}

// --------------------------------------------------
// Validação ANTES de montar a bandeira
// --------------------------------------------------
modded class TerritoryFlagKit
{
    override bool CanBePlaced(Man player, vector position)
	{
		if (!Flag_Base.BaseStarterDataInitialized)
		{
			Flag_Base.InitBaseStarterData();
			Flag_Base.BaseStarterDataInitialized = true;
		}

		PlayerBase pb = PlayerBase.Cast(player);
		if (!pb)
			return super.CanBePlaced(player, position);

		// ===============================
        // EXCEÇÃO ADM
        // ===============================
        if (Flag_Base.IsAdminExempt(pb))
        {
            Print("[ZHFBaseKit] ADM detectado — regras de BaseKit ignoradas.");
            return true;
        }

		string flaggedId = pb.GetIdentity().GetPlainId() + "987";

		if (Flag_Base.FlaggedPlayers.Find(flaggedId) > -1)
		{
			pb.MessageAction("[ZHF] Você só pode ter uma bandeira ativa! Desmonte a atual para colocar outra.");
			return false;
		}

		return super.CanBePlaced(player, position);
	}
}

// --------------------------------------------------
// Modificação da ação de desmontagem do Expansion
// --------------------------------------------------
modded class ExpansionActionDismantleFlag
{
    override void OnFinishProgressServer(ActionData action_data)
    {
        super.OnFinishProgressServer(action_data);

        PlayerBase player = PlayerBase.Cast(action_data.m_Player);
        if (!player)
            return;

        EntityAI targetObj = action_data.m_Target.GetObject();
        TerritoryFlag flag = TerritoryFlag.Cast(targetObj);

        if (flag)
        {
            vector pos = flag.GetPosition();
            string key = TerritoryFlag.ZHF_PosToKey(pos);

            int now = GetGame().GetTime();

            // Registra posição desmontada pelo player
            TerritoryFlag.zhf_PlayerDismantledMap.Insert(key, now);

            Print("[ZHFBaseKit] Marca desmontada pelo player: " + key + " (ts=" + now + ")");
        }

        // mantém a lógica original
        Flag_Base.OnExpansionFlagDismantled(player);
    }
}
