dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "empire_building",
   name = "empire_weaponsmithy",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Weapon Smithy"),
   directory = dirname,
   icon = dirname .. "menu.png",
   size = "big",

   buildcost = {
      log = 2,
      granite = 2,
      marble = 2,
      marble_column = 3
   },
   return_on_dismantle = {
      log = 1,
      granite = 1,
      marble = 2,
      marble_column = 1
   },

   animations = {
      idle = {
         template = "idle_??",
         directory = dirname,
         hotspot = { 44, 61 },
      },
      build = {
         template = "build_??",
         directory = dirname,
         hotspot = { 44, 61 },
      },
      working = {
         template = "working_??",
         directory = dirname,
         hotspot = { 44, 61 },
         fps = 2
      },
   },

   aihints = {
      prohibited_till = 900
   },

   working_positions = {
      empire_weaponsmith = 1
   },

   inputs = {
      iron = 8,
      gold = 8,
      coal = 8,
      planks = 8
   },
   outputs = {
      "spear_wooden",
      "spear",
      "spear_advanced",
      "spear_heavy",
      "spear_war"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_spear_wooden",
            "call=produce_spear",
            "call=produce_spear_advanced",
            "call=produce_spear_heavy",
            "call=produce_spear_war",
            "return=skipped"
         }
      },
      produce_spear_wooden = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a wooden spear because ...
         descname = _"forging a wooden spear",
         actions = {
            "return=skipped unless economy needs spear_wooden",
            "sleep=20000",
            "consume=planks",
            "playFX=sound/smiths smith 192",
            "animate=working 30000",
            "playFX=sound/smiths sharpening 120",
            "produce=spear_wooden"
         }
      },
      produce_spear = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a spear because ...
         descname = _"forging a spear",
         actions = {
            "return=skipped unless economy needs spear",
            "sleep=32000",
            "consume=coal iron planks",
            "playFX=sound/smiths smith 192",
            "animate=working 45000",
            "playFX=sound/smiths sharpening 120",
            "produce=spear"
         }
      },
      produce_spear_advanced = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging an advanced spear because ...
         descname = _"forging an advanced spear",
         actions = {
            "return=skipped unless economy needs spear_advanced",
            "sleep=32000",
            "consume=coal iron:2 planks",
            "playFX=sound/smiths smith 192",
            "animate=working 45000",
            "playFX=sound/smiths sharpening 120",
            "produce=spear_advanced"
         }
      },
      produce_spear_heavy = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a heavy spear because ...
         descname = _"forging a heavy spear",
         actions = {
            "return=skipped unless economy needs spear_heavy",
            "sleep=32000",
            "consume=coal:2 gold iron planks",
            "playFX=sound/smiths smith 192",
            "animate=working 45000",
            "playFX=sound/smiths sharpening 120",
            "produce=spear_heavy"
         }
      },
      produce_spear_war = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a war spear because ...
         descname = _"forging a war spear",
         actions = {
            "return=skipped unless economy needs spear_war",
            "sleep=32000",
            "consume=coal:2 gold iron:2 planks",
            "playFX=sound/smiths smith 192",
            "animate=working 45000",
            "playFX=sound/smiths sharpening 120",
            "produce=spear_war"
         }
      },
   },
}
