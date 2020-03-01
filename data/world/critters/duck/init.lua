dirname = path.dirname(__file__)

animations = {}

add_directional_animation(animations, "walk", dirname, "walk", {5, 10})

world:new_critter_type{
   name = "duck",
   descname = _ "Duck",
   editor_category = "critters_aquatic",
   attributes = { "swimming" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
   spritesheets = {
      idle = {
         directory = dirname,
         basename = "idle",
         frames = 8,
         rows = 4,
         columns = 2,
         hotspot = { 5, 7 }
      }
   }
}
