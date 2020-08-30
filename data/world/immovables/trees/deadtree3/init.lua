wl.World():new_immovable_type{
   name = "deadtree3",
   descname = _ "Dead Tree",
   size = "none",
   animation_directory = path.dirname(__file__),
   programs = {
      main = {
         "animate=idle duration:20s",
         "remove=chance:6.25%"
      }
   },
   animations = {
      idle = {
         hotspot = { 12, 60 }
      },
   }
}
