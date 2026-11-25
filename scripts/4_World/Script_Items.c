//1 Control item

// --------------------------------------------------
// Classe de configuração de item (declaração GLOBAL)
// --------------------------------------------------
class BaseItemConfig
{
    string ClassName;
    int Count;
}

// Classe para carregar JSON completo com webhook
class StarterDataJSON
{
	string Webhook_Reg_Base;
    string Webhook_Base_Abandonada;
	string MapName;
	float CleanupRadius; 
	int UpdateTime;
	
    ref array<string> AdminSteamIDs; 
	ref array<ref BaseItemConfig> Items;
}

// --------------------------------------------------
// NOVO: Estrutura para novo arquivo de controle
// --------------------------------------------------
class ZHF_FlagData
{
	string SteamID;
    string Name;
	float PosX;
	float PosY;
	float PosZ;
}

// --------------------------------------------------
// Modificado: Flag_Base
// --------------------------------------------------
modded class Flag_Base extends ItemBase
{
    static string FILE_PATH_PLAYERS;
    static string FILE_PATH_ITEMS;
    static string FILE_PATH_FLAGS;

    // --------------------------------------------------
    // NOVO: arquivo auxiliar (não interfere nos existentes)
    // --------------------------------------------------
    static string FILE_PATH_FLAGSDATA;
    static ref array<ref ZHF_FlagData> FlagsData = new array<ref ZHF_FlagData>;

    static ref array<string> RewardedPlayers = new array<string>;
    static ref array<string> FlaggedPlayers = new array<string>;
    static ref array<ref BaseItemConfig> StarterItems = new array<ref BaseItemConfig>;

    static string DiscordWebhookURL;
    static string DiscordWebhookAbandoned;
	static string MapName;
    static float  CleanupRadius = 1.0;
	static int  UpdateTime = 5; 
	static bool ConfigReloadLoopStarted = false;
    static bool BaseStarterDataInitialized = false;

    static ref array<string> AdminSteamIDs = new array<string>(); // <-- ADICIONADO

    // --------------------------------------------------
    // Verifica se player é ADM
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
    // Inicializa dados da base e arquivos
    // --------------------------------------------------
    static void InitBaseStarterData()
    {
        if (!GetGame().IsServer())
            return;

        string folder = GetHFProfileFolder();

        FILE_PATH_PLAYERS = folder + "Player_Items.json";
        FILE_PATH_FLAGS   = folder + "Player_Flag.json";
        FILE_PATH_ITEMS   = folder + "Config.json";

        // --------------------------------------------------
        // NOVO: caminho do novo arquivo
        // --------------------------------------------------
        FILE_PATH_FLAGSDATA = folder + "FlagsData.json";

        // ------------------------------
        // Criação de Player_Items.json
        // ------------------------------
        if (!FileExist(FILE_PATH_PLAYERS))
        {
            RewardedPlayers = new array<string>;
            JsonFileLoader<array<string>>.JsonSaveFile(FILE_PATH_PLAYERS, RewardedPlayers);
            Print("[ZHFBaseKit] Criado novo arquivo Player_Items.json.");
        }
        else
        {
            JsonFileLoader<array<string>>.JsonLoadFile(FILE_PATH_PLAYERS, RewardedPlayers);
            Print("[ZHFBaseKit] Jogadores carregados: " + RewardedPlayers.Count().ToString());
        }

        // ------------------------------
        // Criação de Player_Flag.json
        // ------------------------------
        if (!FileExist(FILE_PATH_FLAGS))
        {
            FlaggedPlayers = new array<string>;
            JsonFileLoader<array<string>>.JsonSaveFile(FILE_PATH_FLAGS, FlaggedPlayers);
            Print("[ZHFBaseKit] Criado novo arquivo Player_Flag.json.");
        }
        else
        {
            JsonFileLoader<array<string>>.JsonLoadFile(FILE_PATH_FLAGS, FlaggedPlayers);
            Print("[ZHFBaseKit] Bandeiras carregadas: " + FlaggedPlayers.Count().ToString());
        }

        // --------------------------------------------------
        // NOVO: criação / carregamento FlagsData.json
        // --------------------------------------------------
        if (!FileExist(FILE_PATH_FLAGSDATA))
        {
            FlagsData = new array<ref ZHF_FlagData>;
            JsonFileLoader<array<ref ZHF_FlagData>>.JsonSaveFile(FILE_PATH_FLAGSDATA, FlagsData);
            Print("[ZHFBaseKit] Criado novo arquivo FlagsData.json.");
        }
        else
        {
            JsonFileLoader<array<ref ZHF_FlagData>>.JsonLoadFile(FILE_PATH_FLAGSDATA, FlagsData);
            Print("[ZHFBaseKit] FlagsData carregado: " + FlagsData.Count().ToString());
        }

        // ------------------------------
        // Criação de Config.json
        // ------------------------------
        if (!FileExist(FILE_PATH_ITEMS))
        {
            StarterItems = new array<ref BaseItemConfig>;
            StarterItems.Insert(CreateItemConfig("FenceKit", 1));
            StarterItems.Insert(CreateItemConfig("ExpansionWallKit", 1));
            StarterItems.Insert(CreateItemConfig("Pliers", 1));
            StarterItems.Insert(CreateItemConfig("Handsaw", 1));
            StarterItems.Insert(CreateItemConfig("Shovel", 1));
            StarterItems.Insert(CreateItemConfig("Hammer", 1));
            StarterItems.Insert(CreateItemConfig("WoodAxe", 1));
            StarterItems.Insert(CreateItemConfig("MetalWire", 2));
            StarterItems.Insert(CreateItemConfig("Whetstone", 2));
            StarterItems.Insert(CreateItemConfig("ExpansionCodeLock", 2));
            StarterItems.Insert(CreateItemConfig("NailBox", 3));
            StarterItems.Insert(CreateItemConfig("WoodenPlank", 6));
            StarterItems.Insert(CreateItemConfig("WoodenLog", 6));

            StarterDataJSON data = new StarterDataJSON;
            data.Items = StarterItems;
            data.Webhook_Reg_Base = "COLE_AQUI_SEU_WEBHOOK";
            data.Webhook_Base_Abandonada = "COLE_AQUI_SEU_WEBHOOK_ABANDONADA";
			data.MapName = "chernarusplus";
            data.CleanupRadius = 1.0; 
			data.UpdateTime = 5;
            data.AdminSteamIDs = new array<string>(); 

            JsonFileLoader<StarterDataJSON>.JsonSaveFile(FILE_PATH_ITEMS, data);
            Print("[ZHFBaseKit] Config.json criado com itens padrão.");
        }

        // ------------------------------
        // Carregar Config.json existente
        // ------------------------------
        StarterDataJSON loadedData = new StarterDataJSON;
        JsonFileLoader<StarterDataJSON>.JsonLoadFile(FILE_PATH_ITEMS, loadedData);

        if (loadedData.CleanupRadius <= 0)
            loadedData.CleanupRadius = 1.0;

		if (loadedData.UpdateTime <= 0)
            loadedData.UpdateTime = 5;

        if (!loadedData.Items)
            loadedData.Items = new array<ref BaseItemConfig>;

        if (loadedData.Webhook_Reg_Base == string.Empty)
            loadedData.Webhook_Reg_Base = "COLE_AQUI_SEU_WEBHOOK";

        if (loadedData.Webhook_Base_Abandonada == string.Empty)
            loadedData.Webhook_Base_Abandonada = "COLE_AQUI_SEU_WEBHOOK_ABANDONADA";
		
		if (loadedData.MapName == string.Empty)
            loadedData.MapName = "chernarusplus";

        if (!loadedData.AdminSteamIDs)
            loadedData.AdminSteamIDs = new array<string>(); // <-- ADICIONADO

        StarterItems            = loadedData.Items;
        DiscordWebhookURL       = loadedData.Webhook_Reg_Base;
        DiscordWebhookAbandoned = loadedData.Webhook_Base_Abandonada;
		MapName 				= loadedData.MapName;
        CleanupRadius 			= loadedData.CleanupRadius;
		UpdateTime 				= loadedData.UpdateTime;
        AdminSteamIDs 			= loadedData.AdminSteamIDs; // <-- ADICIONADO

        Print("[ZHFBaseKit] Itens carregados: " + StarterItems.Count());
        Print("[ZHFBaseKit] Webhook Base: " + DiscordWebhookURL);
        Print("[ZHFBaseKit] Webhook Abandonada: " + DiscordWebhookAbandoned);
    }
	
	// --------------------------------------------------
    // Recarrega o Config.json
    // --------------------------------------------------
    static void ReloadConfig()
    {
        if (!GetGame().IsServer())
            return;

        if (FILE_PATH_ITEMS == string.Empty)
            FILE_PATH_ITEMS = GetHFProfileFolder() + "Config.json";

        if (!FileExist(FILE_PATH_ITEMS))
            return;

        StarterDataJSON data = new StarterDataJSON;
        JsonFileLoader<StarterDataJSON>.JsonLoadFile(FILE_PATH_ITEMS, data);

        if (data.Items && data.Items.Count() > 0)
            StarterItems = data.Items;

        if (data.Webhook_Reg_Base != string.Empty)
            DiscordWebhookURL = data.Webhook_Reg_Base;

        if (data.Webhook_Base_Abandonada != string.Empty)
            DiscordWebhookAbandoned = data.Webhook_Base_Abandonada;

        if (data.MapName != string.Empty)
            MapName = data.MapName;

        if (data.CleanupRadius > 0)
            CleanupRadius = data.CleanupRadius;

        if (data.UpdateTime > 0)
            UpdateTime = data.UpdateTime;

        if (data.AdminSteamIDs)
            AdminSteamIDs = data.AdminSteamIDs;

        Print("[ZHFBaseKit] Config.json recarregado. Itens: " + StarterItems.Count() + " | UpdateTime: " + UpdateTime + " min");
    }

    static void StartConfigReloadLoop()
    {
        if (!GetGame().IsServer() || ConfigReloadLoopStarted)
            return;

        ConfigReloadLoopStarted = true;
        ScheduleConfigReload();
    }

    static void ScheduleConfigReload()
    {
        int minutes = UpdateTime;
        if (minutes < 1)
            minutes = 5;

        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Flag_Base.ConfigReloadTick, minutes * 60 * 1000, false);
    }

    static void ConfigReloadTick()
    {
        ReloadConfig();
        ScheduleConfigReload();
    }

    // --------------------------------------------------
    // NOVO: função de save do novo arquivo
    // --------------------------------------------------
    static void SaveFlagsData()
    {
        if (GetGame().IsServer())
            JsonFileLoader<array<ref ZHF_FlagData>>.JsonSaveFile(FILE_PATH_FLAGSDATA, FlagsData);
    }

    static ref BaseItemConfig CreateItemConfig(string name, int count)
    {
        BaseItemConfig cfg = new BaseItemConfig();
        cfg.ClassName = name;
        cfg.Count = count;
        return cfg;
    }

    static void SaveBaseStarterData()
    {
        if (GetGame().IsServer())
        {
            JsonFileLoader<array<string>>.JsonSaveFile(FILE_PATH_PLAYERS, RewardedPlayers);
            JsonFileLoader<array<string>>.JsonSaveFile(FILE_PATH_FLAGS, FlaggedPlayers);
        }
    }

    // --------------------------------------------------
    // Cria pastas HF/BaseKit
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
    // Quando a bandeira é colocada
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

        string playerId = player.GetIdentity().GetPlainId();
        string flaggedId = playerId + "987";

        // ------------------------------
        // VALIDAÇÃO DO BARRIL (ID PURO)
        // ------------------------------
        if (RewardedPlayers.Find(playerId) > -1)
        {
            player.MessageAction("[ZHF] Você já recebeu seu kit inicial de construção.");
        }
        else
        {
            RewardedPlayers.Insert(playerId);
            SaveBaseStarterData();

            array<Object> nearbyObjects = new array<Object>();
            GetGame().GetObjectsAtPosition(player.GetPosition(), 3.0, nearbyObjects, null);
            foreach (Object obj : nearbyObjects)
            {
                if (obj && obj.GetType() == "ZHF_Barrel_Red")
                    return;
            }

            vector playerPos = player.GetPosition();
            vector forward = player.GetDirection();
            forward[1] = 0;
            forward = forward.Normalized();
            vector chestPos = playerPos + forward * 0.2;
            chestPos[1] = playerPos[1];

            EntityAI chest = EntityAI.Cast(GetGame().CreateObject("ZHF_Barrel_Red", chestPos));
			if (!chest) return;
			chest.SetPosition(chestPos);
			chest.SetOrientation("0 0 0");

			Barrel_ColorBase barrel = Barrel_ColorBase.Cast(chest);
			if (barrel)
				barrel.Open();

			foreach (BaseItemConfig cfg : StarterItems)
			{
				if (cfg && cfg.ClassName != "")
					AddItemToChest(chest, cfg.ClassName, cfg.Count);
			}

			if (barrel)
				barrel.Close();

			player.MessageAction("[ZHF] Parabéns! Você recebeu seu kit de construção. Tecle B para mais modelos de construção.");
        }

        // ------------------------------
        // REGISTRO DA BANDEIRA (ID + 987)
        // ------------------------------
        if (FlaggedPlayers.Find(flaggedId) == -1)
        {
            FlaggedPlayers.Insert(flaggedId);
            SaveBaseStarterData();
        }

        // ------------------------------
		// REGISTRO NO FlagsData.json (NOVO)
		// ------------------------------
		ZHF_FlagData fd = new ZHF_FlagData;
		fd.SteamID = playerId;
		fd.Name = player.GetIdentity().GetName(); // <-- pega o nome do player
		vector pos = parent.GetPosition();
		fd.PosX = pos[0];
		fd.PosY = pos[1];
		fd.PosZ = pos[2];

		FlagsData.Insert(fd);
		SaveFlagsData();

    }

    void AddItemToChest(EntityAI chest, string itemName, int count = 1)
	{
		if (!chest) return;

		for (int i = 0; i < count; i++)
		{
			EntityAI item = chest.GetInventory().CreateEntityInCargo(itemName);

			if (!item)
			{
				Print("[ZHFBaseKit] Sem espaço no barril para: " + itemName + " (" + (i + 1) + "/" + count + ")");

				vector dropPos = chest.GetPosition() + Vector(Math.RandomFloat(-1, 1), 0, Math.RandomFloat(-1, 1));
				dropPos[1] = chest.GetPosition()[1];
				item = EntityAI.Cast(GetGame().CreateObjectEx(itemName, dropPos, ECE_PLACE_ON_SURFACE));
			}
		}
	}

    PlayerBase GetNearestPlayer(vector pos)
    {
        array<Man> players = new array<Man>;
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
}
