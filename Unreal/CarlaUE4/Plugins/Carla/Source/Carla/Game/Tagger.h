// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/rpc/ObjectLabel.h>
#include <compiler/enable-ue4-macros.h>

#include "Tagger.generated.h"

namespace crp = carla::rpc;

/// Sets actors' custom depth stencil value for semantic segmentation according
/// to their meshes.
///
/// Non-static functions present so it can be dropped into the scene for testing
/// purposes.
UCLASS()
class CARLA_API ATagger : public AActor
{
  GENERATED_BODY()

public:

  /// Set the tag of an actor.
  ///
  /// If bTagForSemanticSegmentation true, activate the custom depth pass. This
  /// pass is necessary for rendering the semantic segmentation. However, it may
  /// add a performance penalty since occlusion doesn't seem to be applied to
  /// objects having this value active.
  static void TagActor(const AActor &Actor, bool bTagForSemanticSegmentation);


  /// Set the tag of every actor in level.
  ///
  /// If bTagForSemanticSegmentation true, activate the custom depth pass. This
  /// pass is necessary for rendering the semantic segmentation. However, it may
  /// add a performance penalty since occlusion doesn't seem to be applied to
  /// objects having this value active.
  static void TagActorsInLevel(UWorld &World, bool bTagForSemanticSegmentation);

  /// Retrieve the tag of an already tagged component.
  static crp::CityObjectLabel GetTagOfTaggedComponent(const UPrimitiveComponent &Component)
  {
    return static_cast<crp::CityObjectLabel>(Component.CustomDepthStencilValue);
  }

  /// Retrieve the tags of an already tagged actor. CityObjectLabel::None is
  /// not added to the array.
  static void GetTagsOfTaggedActor(const AActor &Actor, TSet<crp::CityObjectLabel> &Tags);

  /// Return true if @a Component has been tagged with the given @a Tag.
  static bool MatchComponent(const UPrimitiveComponent &Component, crp::CityObjectLabel Tag)
  {
    return (Tag == GetTagOfTaggedComponent(Component));
  }

  /// Retrieve the tags of an already tagged actor. CityObjectLabel::None is
  /// not added to the array.
  static FString GetTagAsString(crp::CityObjectLabel Tag);

  /// Method that computes the label corresponding to a folder path
  static crp::CityObjectLabel GetLabelByFolderName(const FString &String);

  /// Method that computes the label corresponding to an specific object
  /// using the folder path in which it is stored
  template <typename T>
  static crp::CityObjectLabel GetLabelByPath(const T *Object) {
    const FString Path = Object->GetPathName();
    TArray<FString> StringArray;
    Path.ParseIntoArray(StringArray, TEXT("/"), false);

    return (StringArray.Num() > 4 ? GetLabelByFolderName(StringArray[4]) : crp::CityObjectLabel::None);
  }

  /// Method that computes the instance label corresponding to a vehicle labled with 10u
  /// using GetLabelByPath and in internal instance counter, maximum lables for overflow 5000
  template <typename T>
  static crp::CityObjectLabel GetInstanceLabel(const T *Object, const int actorID) {
    const auto label = GetLabelByPath(Object);

    if ( ((uint32) label) == 10u) {
        if( instanceActorMap.find(actorID) == instanceActorMap.end()) {
                instanceCounter++;
                instanceActorMap.insert(Int_Pair(actorID, instanceCounter));
        }
        // bounded by https://docs.unrealengine.com/en-US/API/Runtime/Engine/Components/UPrimitiveComponent/SetCustomDepthStencilValue/index.html
        // UPrimitiveComponent::SetCustomDepthStencilValue
        // to range from 0-255
        int tmp = (instanceActorMap.at(actorID) % 256);
        if (tmp < 23) {
            const auto labelb =(crp::CityObjectLabel) (tmp + 23);
            return labelb;
        } else {
            const auto labela = (crp::CityObjectLabel) tmp;
            return labela;
        }
    }
    return label;
  }

  static void SetStencilValue(UPrimitiveComponent &Component,
    const crp::CityObjectLabel &Label, const bool bSetRenderCustomDepth);

  ATagger();

protected:

#if WITH_EDITOR
  virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

private:

  UPROPERTY(Category = "Tagger", EditAnywhere)
  bool bTriggerTagObjects = false;

  UPROPERTY(Category = "Tagger", EditAnywhere)
  bool bTagForSemanticSegmentation = false;

  // instance segmentation values
  static uint32 instanceCounter;
  static std::map<int, uint32> instanceActorMap;
  typedef std::pair<int, uint32> Int_Pair;
};
