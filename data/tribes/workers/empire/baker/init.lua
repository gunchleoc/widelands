dirname = path.dirname(__file__)

tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_baker",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Baker"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1,
      bread_paddle = 1
   },

   animations = {
      idle = {
         directory = dirname,
         hotspot = { 5, 23 }
      }
   },
   spritesheets = {
      walk = {
         directory = dirname,
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 12, 24 }
      },
      walkload = {
         directory = dirname,
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 12, 24 }
      }
   }
}
