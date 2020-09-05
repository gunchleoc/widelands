push_textdomain("world")

wl.World():new_critter_type{
   name = "marten",
   descname = _ "Marten",
   animation_directory = path.dirname(__file__),
   size = 2,
   reproduction_rate = 50,
   appetite = 20,
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
         hotspot = { 11, 11 }
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 11, 11 }
      },
      walk = {
         fps = 50,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 15, 14 }
      }
   }
}

pop_textdomain()
