wl.World():new_immovable_type{
   name = "deadtree5",
   descname = _ "Dead Tree",
   size = "none",
   animation_directory = path.dirname(__file__),
   programs = {
      main = {
         "animate=idle duration:20s",
         "remove=chance:4.69%"
      }
   },
   animations = {
      idle = {
         hotspot = { 6, 28 }
      },
   }
}
