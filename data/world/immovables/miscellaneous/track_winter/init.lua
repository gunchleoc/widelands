wl.World():new_immovable_type{
   name = "track_winter",
   -- TRANSLATORS: This track is made of footprints in the snow
   descname = _ "Track",
   size = "none",
   animation_directory = path.dirname(__file__),
   programs = {},
   animations = {
      idle = {
         hotspot = { 27, 2 }
      },
   }
}
