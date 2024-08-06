/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "VehicleActions.h"
#include "ItemVisitors.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "Vehicle.h"
#include "BattlegroundIC.h"

// TODO methods to enter/exit vehicle should be added to BGTactics or MovementAction (so that we can better control whether bot is in vehicle, eg: get out of vehicle to cap flag, if we're down to final boss, etc),
//      right now they will enter vehicle based only what's available here, then they're stuck in vehicle until they die (LeaveVehicleAction doesnt do much seeing as they, or another bot, will get in immediately after exit)
bool EnterVehicleAction::Execute(Event event)
{
    // do not switch vehicles yet
    if (bot->GetVehicle())
        return false;

    GuidVector npcs = AI_VALUE(GuidVector, "nearest vehicles");
    for (GuidVector::iterator i = npcs.begin(); i != npcs.end(); i++)
    {
        Unit* vehicleBase = botAI->GetUnit(*i);
        if (!vehicleBase)
            continue;

        // dont let them get in the cannons as they'll stay forever and do nothing useful
        // dont let them in catapult they cant use them at all
        if (NPC_KEEP_CANNON == vehicleBase->GetEntry() || NPC_CATAPULT == vehicleBase->GetEntry())
            continue;

        if (!vehicleBase->IsFriendlyTo(bot))
            continue;

        if (!vehicleBase->GetVehicleKit()->GetAvailableSeatCount())
            continue;

        // this will avoid adding passengers (which dont really do much for the IOC vehicles which is the only place this code is used)
        if (vehicleBase->GetVehicleKit()->IsVehicleInUse())
            continue;

        //if (fabs(bot->GetPositionZ() - vehicleBase->GetPositionZ()) < 20.0f)

        //if (sServerFacade->GetDistance2d(bot, vehicle) > 100.0f)
        //    continue;

        if (sServerFacade->GetDistance2d(bot, vehicleBase) > INTERACTION_DISTANCE)
            return MoveTo(vehicleBase, INTERACTION_DISTANCE - 1.0f);

        bot->EnterVehicle(vehicleBase);

        if (!bot->IsOnVehicle(vehicleBase))
            return false;

        // dismount because bots can enter vehicle on mount
        WorldPacket emptyPacket;
        bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);
        return true;
    }

	return false;
}

bool LeaveVehicleAction::Execute(Event event)
{
    Vehicle* myVehicle = bot->GetVehicle();
    if (!myVehicle)
        return false;

    VehicleSeatEntry const* seat = myVehicle->GetSeatForPassenger(bot);
    if (!seat || !seat->CanEnterOrExit())
        return false;

    WorldPacket p;
    bot->GetSession()->HandleRequestVehicleExit(p);

    return true;
}
