#pragma once

struct AIReaction
{
	AIReaction() = default;
	AIReaction(
		int32 reaction,
		bool consumed,
		int64_t startTime,
		int64_t lifeTimeMs) :
			ReactionEnumType(reaction),
			Consumed(consumed),
			StartTime(startTime),
			LifeTimeMs(lifeTimeMs) {}
	
	int32 ReactionEnumType {0};
	bool Consumed {false};
	int64_t StartTime {0};
	int64_t LifeTimeMs {0};
};
