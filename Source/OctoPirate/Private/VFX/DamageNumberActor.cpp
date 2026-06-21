#include "VFX/DamageNumberActor.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"

ADamageNumberActor::ADamageNumberActor()
{
	PrimaryActorTick.bCanEverTick = true;

	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>("WidgetComponent");
	RootComponent = WidgetComponent;
	WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	WidgetComponent->SetDrawSize(FVector2D(200.f, 50.f));
}

void ADamageNumberActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (WidgetClass)
	{
		WidgetComponent->SetWidgetClass(WidgetClass);
		WidgetComponent->InitWidget();
	}
	
	SetLifeSpan(LifeTime);
}

void ADamageNumberActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	AddActorWorldOffset(FVector(0.0f, 0.0f, FloatUpSpeed * DeltaTime));
}

void ADamageNumberActor::Setup(float DamageAmount)
{
	if (!WidgetComponent) return;
	
	UUserWidget* Widget = WidgetComponent->GetUserWidgetObject();
	if (!Widget) return;
	
	UFunction* Func = Widget->FindFunction(FName("SetDamageNumber"));
	if (Func)
	{
		struct FSetupParams { double Damage; };
		FSetupParams Params { DamageAmount };
		Widget->ProcessEvent(Func, &Params);
	}
}
