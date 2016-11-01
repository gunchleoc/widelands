dirname = path.dirname(__file__)

tribes:new_productionsite_type {
   msgctxt = "barbarians_building",
   name = "barbarians_ax_workshop",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Ax Workshop"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   size = "medium",
   enhancement = "barbarians_warmill",

   enhancement_cost = {
      log = 1,
      blackwood = 1,
      granite = 2,
      grout = 1,
      thatch_reed = 1
   },
   return_on_dismantle_on_enhanced = {
      blackwood = 1,
      granite = 1,
      grout = 1
   },

   spritemap_script = dirname .. "animations.lua",
   aihints = {},

   working_positions = {
      barbarians_blacksmith = 1
   },

   inputs = {
      { name = "coal", amount = 8 },
      { name = "iron", amount = 8 }
   },
   outputs = {
      "ax",
      "ax_sharp",
      "ax_broad"
   },

   programs = {
      work = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _"working",
         actions = {
            "call=produce_ax",
            "call=produce_ax_sharp",
            "call=produce_ax_broad",
            "return=skipped"
         }
      },
      produce_ax = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging an ax because ...
         descname = _"forging an ax",
         actions = {
            "return=skipped unless economy needs ax",
            "sleep=26000",
            "consume=coal iron",
            "play_sound=sound/smiths smith 192",
            "animate=working 22000",
            "play_sound=sound/smiths sharpening 120",
            "sleep=9000",
            "produce=ax"
         }
      },
      produce_ax_sharp = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a sharp ax because ...
         descname = _"forging a sharp ax",
         actions = {
            "return=skipped unless economy needs ax_sharp",
            "consume=coal iron:2",
            "sleep=26000",
            "play_sound=sound/smiths smith 192",
            "animate=working 22000",
            "play_sound=sound/smiths sharpening 120",
            "sleep=9000",
            "produce=ax_sharp"
         }
      },
      produce_ax_broad = {
         -- TRANSLATORS: Completed/Skipped/Did not start forging a broad ax because ...
         descname = _"forging a broad ax",
         actions = {
            "return=skipped unless economy needs ax_broad",
            "consume=coal:2 iron:2",
            "sleep=26000",
            "play_sound=sound/smiths smith 192",
            "animate=working 22000",
            "play_sound=sound/smiths sharpening 120",
            "sleep=9000",
            "produce=ax_broad"
         }
      },
   },
}
