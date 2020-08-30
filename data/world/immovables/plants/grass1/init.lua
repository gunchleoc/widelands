wl.World():new_immovable_type{
   name = "grass1",
   descname = _ "Grass",
   size = "none",
   animation_directory = path.dirname(__file__),
   programs = {},
   animations = {
      idle = {
         hotspot = { 8, 17 },
         sound_effect = {
            path = "sound/animals/frog",
            priority = 0.01
         },
      },
   }
}
