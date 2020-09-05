push_textdomain("world")

wl.World():new_critter_type{
   name = "lynx",
   descname = _ "Lynx",
   animation_directory = path.dirname(__file__),
   size = 2,
   reproduction_rate = 30,
   appetite = 10,
   carnivore = true,

   programs = {
      remove = { "remove" },
   },
   spritesheets = {
      idle = {
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 8, 14 }
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 8, 14 }
      },
      walk = {
         fps = 25,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 11, 21 }
      }
   }
}

pop_textdomain()
