// This file is covered by the LICENSE file in the root of this plugin.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Actor.h"
#include "PauseMenuActor.generated.h"

UCLASS()
class TACTICALVRCORE_API APauseMenuActor : public AActor
{
	GENERATED_BODY()
	
public:	
	/** Sets default values for this actor's properties */
	APauseMenuActor();

    /**
     * Call to focus a Widget from this Menu.
     * @param NewFocusWidget WidgetComponent to focus. If it is not an Widget Component from this menu the function won't do anything.
     */
    UFUNCTION(Category = "Menu", BlueprintCallable, BlueprintImplementableEvent)
    void FocusWidget(UWidgetComponent* NewFocusWidget);

    /**
     * Unused might be necessary if there is a tertiary menu structure
     */
    UFUNCTION(Category = "Menu", BlueprintCallable, BlueprintImplementableEvent)
    void UnfocusWidget(UWidgetComponent* OldFocusWidget);

    /**
     * Returns the currently active widget that the widget interaction component is supposed to interactiv with
     */
    UFUNCTION(Category = "Menu", BlueprintCallable)
    UWidgetComponent* GetCurrentlyActiveWidget() const { return CurrentlyActiveWidget; }
    
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

    UPROPERTY(Category = "Menu", BlueprintReadWrite)
    UWidgetComponent* CurrentlyActiveWidget;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

    UFUNCTION(Category = "Menu", BlueprintImplementableEvent, BlueprintCallable)
    void StartRemoveMenu();


};
