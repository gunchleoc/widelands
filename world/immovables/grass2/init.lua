dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "grass2",
   descname = _ "Grass",
   editor_category = "plants",
   size = "none",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 10, 16 },
      },
   }
}
