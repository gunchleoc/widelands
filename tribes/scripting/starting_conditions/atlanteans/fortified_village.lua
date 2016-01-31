-- =======================================================================
--                    Fortified Village Starting Conditions
-- =======================================================================

include "scripting/infrastructure.lua"

set_textdomain("tribes")

return {
   descname = _ "Fortified Village",
   func =  function(plr, shared_in_start)

   local sf = wl.Game().map.player_slots[plr.number].starting_field
   if shared_in_start then
      sf = shared_in_start
   else
      plr:allow_workers("all")
   end

   local h = plr:place_building("atlanteans_castle", sf, false, true)
   h:set_soldiers{[{0,0,0,0}] = 12}

   if not pcall(function()
      place_building_in_region(plr, "atlanteans_warehouse", sf:region(7), {
         wares = {
            diamond = 7,
            iron_ore = 5,
            quartz = 9,
            granite = 50,
            spider_silk = 9,
            log = 13,
            gold_thread = 6,
            planks = 45,
            spidercloth = 5,
            blackroot = 5,
            blackroot_flour = 12,
            corn = 5,
            cornmeal = 12,
            fish = 3,
            meat = 3,
            water = 12,
            bread_paddle = 2,
            buckets = 2,
            fire_tongs = 2,
            fishing_net = 4,
            hammer = 11,
            hunting_bow = 1,
            milking_tongs = 2,
            hook_pole = 2,
            pick = 8,
            saw = 9,
            scythe = 4,
            shovel = 9,
            tabard = 5,
            trident_light = 5,
         },
         workers = {
            atlanteans_blackroot_farmer = 1,
            atlanteans_builder = 10,
            atlanteans_charcoal_burner = 1,
            atlanteans_carrier = 38,
            atlanteans_fishbreeder = 1,
            atlanteans_geologist = 4,
            atlanteans_miner = 4,
            atlanteans_stonecutter = 2,
            atlanteans_toolsmith = 1,
            atlanteans_woodcutter = 3,
            atlanteans_horse = 5,
         },
         soldiers = {
            [{0,0,0,0}] = 23,
         },
      })

      place_building_in_region(plr, "atlanteans_labyrinth", sf:region(11), {
         wares = {
            atlanteans_bread = 4,
            smoked_fish = 3,
            smoked_meat = 3,
         }
      })

      place_building_in_region(plr, "atlanteans_dungeon", sf:region(11), {
         wares = {atlanteans_bread = 4, smoked_fish = 3, smoked_meat = 3}
      })

      place_building_in_region(plr, "atlanteans_armorsmithy", sf:region(11), {
         wares = { coal=4, gold =4 }
      })
      place_building_in_region(plr, "atlanteans_toolsmithy", sf:region(11), {
         wares = { log = 6 }
      })
      place_building_in_region(plr, "atlanteans_weaponsmithy", sf:region(11), {
         wares = { coal = 8, iron = 8 }
      })

      place_building_in_region(plr, "atlanteans_sawmill", sf:region(11), {
         wares = { log = 1 }
      })
   end) then
      plr:send_message(
         -- TRANSLATORS: Short for "Not enough space"
         _"No Space",
         rt(p(_([[Some of your starting buildings didn’t have enough room and weren’t built. You are at a disadvantage with this; consider restarting this map with a fair starting condition.]]))),
         {popup=true, heading=_"Not enough space"}
      )
   end
end
}
