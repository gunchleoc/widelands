dirname = path.dirname(__file__)

animations = {}

add_directional_animation(animations, "walk", dirname, "walk", {24, 24}, 4)

world:new_critter_type{
   name = "brownbear",
   descname = _ "Brown bear",
   editor_category = "critters_carnivores",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
   spritesheets = {
      idle = {
         directory = dirname,
         basename = "idle",
         fps = 20,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 20, 15 }
      }
   }
}
