push_textdomain("world")

wl.World():new_critter_type{
   name = "fox",
   descname = _ "Fox",
   animation_directory = path.dirname(__file__),
   size = 4,
   reproduction_rate = 80,
   appetite = 70,
   carnivore = true,

   programs = {
      remove = { "remove" },
   },
   spritesheets = {
      idle = {
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 10, 13 },
         sound_effect = {
            path = "sound/animals/coyote",
            priority = 0.01
         }
      },
      eating = {
         basename = "idle", -- TODO(Nordfriese): Make animation
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 10, 13 },
      },
      walk = {
         fps = 25,
         frames = 20,
         rows = 5,
         columns = 4,
         directional = true,
         hotspot = { 11, 14 }
      }
   }
}

pop_textdomain()
