class CfgPatches
{
    class WelcomePackBase
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data", "DZ_Scripts", "DZ_Gear_Consumables"};
    };
};

class CfgMods
{
    class WelcomePackBaseScripts
    {
        dir="WelcomePackBase";
        picture="";
        action="";
        hideName=0;
        hidePicture=1;
        name="Welcome Pack Base";
        credits="HF";
        author="HF";
        authorID="0";
        version="1.0";
        extra=0;
        type="mod";
        dependencies[]=
        {
            "World", "Mission"
        };
        class defs
        {
            class worldScriptModule
            {
                value="";
                files[]=
                {
                    "WelcomePackBase/scripts/4_World"
                };
            };
			
			class missionScriptModule
            {
                value="";
                files[]=
                {
                    "WelcomePackBase/scripts/5_Mission"
                };
            };
        };
    };
};
