push_textdomain("world")

wl.World():new_critter_type{
   name = "wisent",
   descname = _ "Wisent",
   animation_directory = path.dirname(__file__),
   size = 10,
   reproduction_rate = 20,
   appetite = 50,
   herbivore = {"field"},

   programs = {
      remove = { "remove" },
   },
   spritesheets = {
      idle = {
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 14, 27 }
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 14, 27 }
      },
      walk = {
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 24, 32 }
      }
   }
}

pop_textdomain()
