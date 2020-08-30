wl.World():new_immovable_type{
   name = "grass3",
   descname = _ "Grass",
   size = "none",
   animation_directory = path.dirname(__file__),
   programs = {},
   animations = {
      idle = {
         hotspot = { 9, 11 },
         sound_effect = {
            path = "sound/animals/frog",
            priority = 0.01
         },
      },
   }
}
