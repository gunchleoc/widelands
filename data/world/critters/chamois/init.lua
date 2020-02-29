dirname = path.dirname(__file__)

animations = {
   idle = {
      directory = dirname,
      basename = "idle",
      hotspot = { 11, 13 },
      fps = 10,
   },
}

add_directional_animation(animations, "walk", dirname, "walk", {11, 20}, 20)

world:new_critter_type{
   name = "chamois",
   descname = _ "Chamois",
   editor_category = "critters_herbivores",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}
