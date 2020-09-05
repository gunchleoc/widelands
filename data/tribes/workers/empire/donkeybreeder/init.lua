push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_worker_type {
   name = "empire_donkeybreeder",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Donkey Breeder"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1
   },

   animations = {
      idle = {
         hotspot = { 29, 22 },
      }
   },
   spritesheets = {
      walk = {
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 26, 20 }
      }
   }
}

pop_textdomain()
