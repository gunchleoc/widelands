dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_lime_burner",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Lime-Burner"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      barbarians_carrier = 1
   },

   animations = {
      idle = {
         directory = dirname,
         hotspot = { 4, 21 },
      }
   },
   spritesheets = {
      walk = {
         directory = dirname,
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 7, 21 }
      },
      walkload = {
         directory = dirname,
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 8, 24 }
      }
   }
}
