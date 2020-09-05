function mission_thread()
   sleep(100)
   -- Back home
   include "map:scripting/starting_conditions.lua"
   p1:hide_fields(wl.Game().map.player_slots[1].starting_field:region(13),true)
   scroll_to_field(wl.Game().map.player_slots[1].starting_field)

   sleep(100)
   reveal_concentric(p1, wl.Game().map.player_slots[1].starting_field, 13)
   sleep(400)

   -- Check for trees and remove them
   local fields = {{12,0},          -- Buildspace
                   {12,1},          -- Flag of building
                   {12,2}, {11,2},  -- Roads ...
                   {10,2}, {9,2},
                   {8,2}, {7,1},
                   {7,0},}
   remove_trees(fields)

   local ship_field = wl.Game().map:get_field(3,62)
   p1:place_ship(ship_field)
   p1:place_ship(ship_field)
   p1:place_ship(ship_field)
   p1:place_ship(ship_field)
   p1:place_ship(ship_field)
   p1:place_ship(ship_field)

end

run(mission_thread)
