push_textdomain("world")

wl.World():new_critter_type{
   name = "deer",
   descname = _ "Deer",
   animation_directory = path.dirname(__file__),
   size = 4,
   reproduction_rate = 40,
   appetite = 70,
   herbivore = {"tree_sapling"},

   programs = {
      remove = { "remove" },
   },

   spritesheets = {
      idle = {
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 1, 10 }
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 1, 10 }
      },
      walk = {
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 10, 19 }
      }
   }
}

pop_textdomain()
