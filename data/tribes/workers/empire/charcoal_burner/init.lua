push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_worker_type {
   name = "empire_charcoal_burner",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Charcoal Burner"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1
   },

   animations = {
      idle = {
         hotspot = { 17, 22 }
      }
   },
   spritesheets = {
      walk = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 18, 22 }
      },
      walkload = {
         fps = 10,
         frames = 10,
         rows = 4,
         columns = 3,
         directional = true,
         hotspot = { 9, 19 }
      }
   }
}

pop_textdomain()
