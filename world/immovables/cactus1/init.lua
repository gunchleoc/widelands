dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "cactus1",
   descname = _ "Cactus",
   editor_category = "plants",
   size = "none",
   attributes = {},
   programs = {},
   animations = {
      idle = {
         template = "idle",
         directory = dirname,
         hotspot = { 10, 71 },
      },
   }
}
