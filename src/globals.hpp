namespace Globals {
    SDK::UHottaGameEngine *getEngine();
    SDK::UHottaGameInstance *getInstance();
    SDK::UWorld *getWorld();
    SDK::UQRSLLocalPlayer *getLocalPlayer();
    SDK::AQRSLPlayerCharacter *getCharacter();
    SDK::APlayerController *getPlayerController();
    SDK::UHottaInventoryComponent *getInventoryComponent();
    SDK::UCheatManager *getCheatManager();
    SDK::UQRSLCheatManager *getQRSLCheatManager();

    template <typename T> T getObject(SDK::UClass *uClass) {
        for (int i = 0; i < SDK::UObject::GObjects->Num(); i++) {
            SDK::UObject *Obj = SDK::UObject::GObjects->GetByIndex(i);

            if (!Obj) {
                continue;
            }

            if (Obj->IsA(uClass)) {
                if (!Obj->IsDefaultObject()) {
                    return (T)Obj;
                }
            }
        }

        return nullptr;
    }

    template <typename T> std::vector<T> getAllObjects(SDK::UClass *uClass) {
        std::vector<T> objects;

        for (int i = 0; i < SDK::UObject::GObjects->Num(); i++) {
            SDK::UObject *Obj = SDK::UObject::GObjects->GetByIndex(i);

            if (!Obj) {
                continue;
            }

            if (Obj->IsA(uClass)) {
                if (!Obj->IsDefaultObject()) {
                    objects.push_back((T)Obj);
                }
            }
        }

        return objects;
    }
} // namespace Globals