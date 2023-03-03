#include "API/ARK/Ark.h"

#define NUM_KIBBLE 12

struct USCS_Node : UObject
{
    UActorComponent*& ComponentTemplateField() { return *GetNativePointerField<UActorComponent**>(this, "USCS_Node.ComponentTemplate"); }
};

struct USimpleConstructionScript : UObject
{
    TArray<USCS_Node*>& RootNodesField() { return *GetNativePointerField<TArray<USCS_Node*>*>(this, "USimpleConstructionScript.RootNodes"); }
};

struct UBlueprintGeneratedClass : UClass
{
    USimpleConstructionScript*& SimpleConstructionScriptField() { return *GetNativePointerField<USimpleConstructionScript**>(this, "UBlueprintGeneratedClass.SimpleConstructionScript"); }
};

UClass* PreservingBinInvClass;
UClass* CompostBinInvClass;
UClass* BeerBarrelInvClass;
UClass* CookingPotInvClass;

UClass* BeerClass;

UClass* KibbleBaseClass;
UClass* KibbleXSmallClass;
UClass* KibbleSmallClass;
UClass* KibbleMediumClass;
UClass* KibbleLargeClass;
UClass* KibbleXLargeClass;
UClass* KibbleSpecialClass;
UClass* KibbleXSmallExClass;
UClass* KibbleSmallExClass;
UClass* KibbleMediumExClass;
UClass* KibbleLargeExClass;
UClass* KibbleXLargeExClass;
UClass* KibbleSpecialExClass;

DECLARE_HOOK(UPrimalInventoryComponent_AddItem,
    UPrimalItem*,                   // return type
    UPrimalInventoryComponent*,     // _this
    FItemNetInfo*,                  // theItemInfo
    bool,                           // bEquipItem
    bool,                           // bAddToSlot
    bool,                           // bDontStack
    FItemNetID*,                    // InventoryInsertAfterItemID
    bool,                           // bShowHUDNotification
    bool,                           // bDontRecalcSpoilingTime
    bool,                           // bForceIncompleteStacking
    AShooterCharacter*,             // OwnerPlayer
    bool,                           // bIgnoreAbsoluteMaxInventory
    bool,                           // bInsertAtItemIDIndexInstead
    bool                            // doVersionCheck
);
UPrimalItem* Hook_UPrimalInventoryComponent_AddItem(
    UPrimalInventoryComponent* _this,
    FItemNetInfo* theItemInfo,
    bool bEquipItem,
    bool bAddToSlot,
    bool bDontStack,
    FItemNetID* InventoryInsertAfterItemID,
    bool bShowHUDNotification,
    bool bDontRecalcSpoilingTime,
    bool bForceIncompleteStacking,
    AShooterCharacter* OwnerPlayer,
    bool bIgnoreAbsoluteMaxInventory,
    bool bInsertAtItemIDIndexInstead,
    bool doVersionCheck)
{
    UClass* itemClass = theItemInfo->ItemArchetypeField().uClass;

    // Show all autocraft blueprints in the Preserving Bin
    // Show all autocraft blueprints in the Compost Bin
    // Show all craftable blueprints in Cooking Pots
    // Show the Beer blueprint in the Beer Barrel
    if (theItemInfo->bIsInitialItem().Get() && ((UPrimalItem*)itemClass->ClassDefaultObjectField())->bAutoCraftBlueprint().Get()) {
        if (_this->IsA(PreservingBinInvClass) ||
            _this->IsA(CompostBinInvClass) ||
            (_this->IsA(CookingPotInvClass) && (((UPrimalItem*)itemClass->ClassDefaultObjectField())->BaseCraftingResourceRequirementsField().Num() != 0)) ||
            (_this->IsA(BeerBarrelInvClass) && (itemClass->IsChildOf(BeerClass))))
        {
            theItemInfo->bIsEngram().Set(false);
            theItemInfo->bHideFromInventoryDisplay().Set(false);
        }
    }

    return UPrimalInventoryComponent_AddItem_original(_this, theItemInfo, bEquipItem, bAddToSlot, bDontStack, InventoryInsertAfterItemID, bShowHUDNotification, bDontRecalcSpoilingTime, bForceIncompleteStacking, OwnerPlayer, bIgnoreAbsoluteMaxInventory, bInsertAtItemIDIndexInstead, doVersionCheck);
}

// Returns true if the UClass is found
bool GetClass(UClass** OutClass, const std::string& ClassName)
{
    *OutClass = Globals::FindClass(ClassName);

    if (*OutClass) {
        return true;
    } else {
        Log::GetLog()->error("Not found: {}", ClassName);
        return false;
    }
}

// Rearrange default kibble blueprints found in a UPrimalInventoryComponent
void RearrangeDefaultKibble(UPrimalInventoryComponent* Inventory)
{
    static UClass* kibbleList[NUM_KIBBLE] = {
        KibbleXSmallClass,
        KibbleSmallClass,
        KibbleMediumClass,
        KibbleLargeClass,
        KibbleXLargeClass,
        KibbleSpecialClass,
        KibbleXSmallExClass,
        KibbleSmallExClass,
        KibbleMediumExClass,
        KibbleLargeExClass,
        KibbleXLargeExClass,
        KibbleSpecialExClass
    };

    int kibbleIdx = 0;
    for (int i = 0; i < Inventory->DefaultInventoryItemsField().Num(); ++i) {
        UClass* itemClass = Inventory->DefaultInventoryItemsField()[i].uClass;
        if ((itemClass->IsChildOf(KibbleBaseClass)) && (((UPrimalItem*)itemClass->ClassDefaultObjectField())->BaseCraftingResourceRequirementsField().Num() != 0)) {
            Inventory->DefaultInventoryItemsField()[i] = kibbleList[kibbleIdx++];
            if (kibbleIdx >= NUM_KIBBLE)
                break;
        }
    }
}

// Called when the server is ready
void OnServerReady()
{
    UBlueprintGeneratedClass* cookingPotStructureClass;
    UBlueprintGeneratedClass* industrialCookingPotStructureClass;
    bool isMissingClass = false;

    isMissingClass = !GetClass(&PreservingBinInvClass, "BlueprintGeneratedClass /Game/PrimalEarth/CoreBlueprints/Inventories/PrimalInventoryBP_PreservingBin.PrimalInventoryBP_PreservingBin_C");
    isMissingClass = !GetClass(&CompostBinInvClass, "BlueprintGeneratedClass /Game/PrimalEarth/CoreBlueprints/Inventories/PrimalInventoryBP_CompostBin.PrimalInventoryBP_CompostBin_C");
    isMissingClass = !GetClass(&BeerBarrelInvClass, "BlueprintGeneratedClass /Game/PrimalEarth/CoreBlueprints/Inventories/PrimalInventoryBP_BeerBarrel.PrimalInventoryBP_BeerBarrel_C");
    isMissingClass = !GetClass(&CookingPotInvClass, "BlueprintGeneratedClass /Game/PrimalEarth/CoreBlueprints/Inventories/PrimalInventoryBP_CookingPot.PrimalInventoryBP_CookingPot_C");

    isMissingClass = !GetClass(&BeerClass, "BlueprintGeneratedClass /Game/PrimalEarth/CoreBlueprints/Resources/PrimalItemResource_Beer.PrimalItemResource_Beer_C");

    isMissingClass = !GetClass(&KibbleBaseClass, "BlueprintGeneratedClass /Game/PrimalEarth/CoreBlueprints/Items/Consumables/PrimalItemConsumable_Kibble_Base.PrimalItemConsumable_Kibble_Base_C");
    isMissingClass = !GetClass(&KibbleXSmallClass, "BlueprintGeneratedClass /Game/PrimalEarth/CoreBlueprints/Items/Consumables/PrimalItemConsumable_Kibble_Base_XSmall.PrimalItemConsumable_Kibble_Base_XSmall_C");
    isMissingClass = !GetClass(&KibbleSmallClass, "BlueprintGeneratedClass /Game/PrimalEarth/CoreBlueprints/Items/Consumables/PrimalItemConsumable_Kibble_Base_Small.PrimalItemConsumable_Kibble_Base_Small_C");
    isMissingClass = !GetClass(&KibbleMediumClass, "BlueprintGeneratedClass /Game/PrimalEarth/CoreBlueprints/Items/Consumables/PrimalItemConsumable_Kibble_Base_Medium.PrimalItemConsumable_Kibble_Base_Medium_C");
    isMissingClass = !GetClass(&KibbleLargeClass, "BlueprintGeneratedClass /Game/PrimalEarth/CoreBlueprints/Items/Consumables/PrimalItemConsumable_Kibble_Base_Large.PrimalItemConsumable_Kibble_Base_Large_C");
    isMissingClass = !GetClass(&KibbleXLargeClass, "BlueprintGeneratedClass /Game/PrimalEarth/CoreBlueprints/Items/Consumables/PrimalItemConsumable_Kibble_Base_XLarge.PrimalItemConsumable_Kibble_Base_XLarge_C");
    isMissingClass = !GetClass(&KibbleSpecialClass, "BlueprintGeneratedClass /Game/PrimalEarth/CoreBlueprints/Items/Consumables/PrimalItemConsumable_Kibble_Base_Special.PrimalItemConsumable_Kibble_Base_Special_C");
    isMissingClass = !GetClass(&KibbleXSmallExClass, "BlueprintGeneratedClass /Game/Extinction/CoreBlueprints/Items/Consumables/PrimalItemConsumable_Kibble_Base_XSmall_EX.PrimalItemConsumable_Kibble_Base_XSmall_EX_C");
    isMissingClass = !GetClass(&KibbleSmallExClass, "BlueprintGeneratedClass /Game/Extinction/CoreBlueprints/Items/Consumables/PrimalItemConsumable_Kibble_Base_Small_EX.PrimalItemConsumable_Kibble_Base_Small_EX_C");
    isMissingClass = !GetClass(&KibbleMediumExClass, "BlueprintGeneratedClass /Game/Extinction/CoreBlueprints/Items/Consumables/PrimalItemConsumable_Kibble_Base_Medium_EX.PrimalItemConsumable_Kibble_Base_Medium_EX_C");
    isMissingClass = !GetClass(&KibbleLargeExClass, "BlueprintGeneratedClass /Game/Extinction/CoreBlueprints/Items/Consumables/PrimalItemConsumable_Kibble_Base_Large_EX.PrimalItemConsumable_Kibble_Base_Large_EX_C");
    isMissingClass = !GetClass(&KibbleXLargeExClass, "BlueprintGeneratedClass /Game/Extinction/CoreBlueprints/Items/Consumables/PrimalItemConsumable_Kibble_Base_XLarge_EX.PrimalItemConsumable_Kibble_Base_XLarge_EX_C");
    isMissingClass = !GetClass(&KibbleSpecialExClass, "BlueprintGeneratedClass /Game/Extinction/CoreBlueprints/Items/Consumables/PrimalItemConsumable_Kibble_Base_Special_EX.PrimalItemConsumable_Kibble_Base_Special_EX_C");

    isMissingClass = !GetClass((UClass**)&cookingPotStructureClass, "BlueprintGeneratedClass /Game/PrimalEarth/Structures/CookingPot.CookingPot_C");
    isMissingClass = !GetClass((UClass**)&industrialCookingPotStructureClass, "BlueprintGeneratedClass /Game/PrimalEarth/Structures/IndustrialCookingPot.IndustrialCookingPot_C");

    if (isMissingClass) {
        Log::GetLog()->error("Initialization failed");
        return;
    }

    // Rearrange the kibble blueprints in Cooking Pot inventories
    for (USCS_Node* node : cookingPotStructureClass->SimpleConstructionScriptField()->RootNodesField())
        if (node->ComponentTemplateField()->IsA(CookingPotInvClass))
            RearrangeDefaultKibble((UPrimalInventoryComponent*)node->ComponentTemplateField());

    // Rearrange the kibble blueprints in Industrial Cooking Pot inventories
    for (USCS_Node* node : industrialCookingPotStructureClass->SimpleConstructionScriptField()->RootNodesField())
        if (node->ComponentTemplateField()->IsA(CookingPotInvClass))
            RearrangeDefaultKibble((UPrimalInventoryComponent*)node->ComponentTemplateField());

    Log::GetLog()->info("Initialization complete");
}

// ArkServerApi hook that triggers once when the server is ready
DECLARE_HOOK(AShooterGameMode_BeginPlay, void, AShooterGameMode*);
void Hook_AShooterGameMode_BeginPlay(AShooterGameMode* _this)
{
    Log::GetLog()->info("Hook_AShooterGameMode_BeginPlay()");
    AShooterGameMode_BeginPlay_original(_this);

    // Call Plugin_ServerReadyInit() for post-"server ready" initialization
    OnServerReady();
}

// Called by ArkServerApi when the plugin is loaded
extern "C" __declspec(dllexport) void Plugin_Init()
{
    Log::Get().Init(PROJECT_NAME);

    ArkApi::GetHooks().SetHook("AShooterGameMode.BeginPlay", Hook_AShooterGameMode_BeginPlay,
        &AShooterGameMode_BeginPlay_original);
    ArkApi::GetHooks().SetHook("UPrimalInventoryComponent.AddItem", Hook_UPrimalInventoryComponent_AddItem,
        &UPrimalInventoryComponent_AddItem_original);

    // If the server is ready, call Plugin_ServerReadyInit() for post-"server ready" initialization
    if (ArkApi::GetApiUtils().GetStatus() == ArkApi::ServerStatus::Ready)
        OnServerReady();
}

// Called by ArkServerApi when the plugin is unloaded
extern "C" __declspec(dllexport) void Plugin_Unload()
{
    ArkApi::GetHooks().DisableHook("AShooterGameMode.BeginPlay", Hook_AShooterGameMode_BeginPlay);
    ArkApi::GetHooks().DisableHook("UPrimalInventoryComponent.AddItem", Hook_UPrimalInventoryComponent_AddItem);
}