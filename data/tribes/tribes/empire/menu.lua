-- This file contains the basic information for all tribes that is
-- needed before a game is loaded or the editor has been started.


dirname = path.dirname(__file__)
set_textdomain("tribes")

return {
   -- Basic information for the Empire tribe
   name = "empire",
   author = _"The Widelands Development Team",
   -- TRANSLATORS: This is a tribe name
   descname = _"Empire",
   tooltip = _"This is the culture of the Roman Empire.",
   icon = dirname .. "images/icon.png",
   script = dirname .. "init.lua",

   starting_conditions = {
      dirname .. "starting_conditions/headquarters.lua";
      dirname .. "starting_conditions/fortified_village.lua";
      dirname .. "starting_conditions/trading_outpost.lua";
      dirname .. "starting_conditions/village.lua";
      dirname .. "starting_conditions/poor_hamlet.lua";
      dirname .. "starting_conditions/struggling_outpost.lua";
      dirname .. "starting_conditions/discovery.lua";
      dirname .. "starting_conditions/new_world.lua";
   }
}
