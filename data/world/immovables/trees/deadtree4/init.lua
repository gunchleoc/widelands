wl.World():new_immovable_type{
   name = "deadtree4",
   descname = _ "Dead Tree",
   size = "none",
   animation_directory = path.dirname(__file__),
   programs = {
      main = {
         "animate=idle duration:20s",
         "remove=chance:7.03%"
      }
   },
   animations = {
      idle = {
         hotspot = { 20, 60 }
      },
   }
}
