/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "VehicleActions.h"
#include "ItemVisitors.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "Vehicle.h"
#include "BattlegroundIC.h"

// TODO enter and exit vehicle should probably be done directly within BGTactics (so that we can control whether bot is in vehicle depending on state of BG, right now they go in if anywhere near vehicle and are basically stuck in there until they die)
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
        if (NPC_KEEP_CANNON == vehicleBase->GetEntry())
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
