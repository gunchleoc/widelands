dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_builder",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Builder"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1,
      hammer = 1
   },

   spritesheets = {
      idle = {
         directory = dirname,
         basename = "idle",
         fps = 10,
         frames = 150,
         rows = 13,
         columns = 12,
         hotspot = { 9, 18 }
      },
      walk = {
         directory = dirname,
         basename = "walk",
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 7, 22 }
      },
      walkload = {
         directory = dirname,
         basename = "walk",
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 7, 22 }
      },
      work = {
         directory = dirname,
         basename = "work",
         fps = 10,
         frames = 92,
         rows = 11,
         columns = 9,
         hotspot = { 9, 22 },
         sound_effect = {
            path = "sound/hammering/hammering",
            priority = 64
         }
      }
   }
}
