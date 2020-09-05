push_textdomain("world")

wl.World():new_critter_type{
   name = "sheep",
   descname = _ "Sheep",
   animation_directory = path.dirname(__file__),
   size = 3,
   reproduction_rate = 40,
   appetite = 90,
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
         hotspot = { 8, 16 },
         sound_effect = {
            path = "sound/farm/sheep",
            priority = 0.01
         }
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 8, 16 }
      },
      walk = {
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 15, 25 }
      }
   }
}

pop_textdomain()
