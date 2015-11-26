dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_piggery",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Piggery"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      log = 2,
      granite = 2,
      marble_column = 2
   },
   return_on_dismantle = {
      log = 1,
      granite = 1,
      marble = 1
   },


   animations = {
      idle = {
         template = "idle_??",
         directory = dirname,
         hotspot = { 82, 74 },
      },
      working = {
         template = "idle_??", -- TODO(GunChleoc): No animation yet.
         directory = dirname,
         hotspot = { 82, 74 },
      },
   },

   aihints = {
      forced_after = 1800
   },

   working_positions = {
      empire_pigbreeder = 1
   },

   inputs = {
      wheat = 7,
      water = 7
   },
   outputs = {
      "meat"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start raising pigs because ...
         descname = _"raising pigs",
         actions = {
            "sleep=25000",
            "return=skipped unless economy needs meat",
            "consume=water wheat",
            "playFX=sound/farm farm_animal 180",
            "animate=working 30000",
            "produce=meat"
         }
      },
   },
}
