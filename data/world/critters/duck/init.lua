push_textdomain("world")

dirname = path.dirname(__file__)

wl.World():new_critter_type{
   name = "duck",
   descname = _ "Duck",
   animation_directory = dirname,
   size = 1,
   reproduction_rate = 10,

   programs = {
      remove = { "remove" },
   },

   animations = {
      walk = {
         hotspot = { 5, 10 },
         directional = true
      }
   },

   spritesheets = {
      idle = {
         frames = 8,
         rows = 4,
         columns = 2,
         hotspot = { 5, 7 },
         sound_effect = {
            path = dirname .. "duck",
            priority = 0.01
         }
      }
   }
}

pop_textdomain()
