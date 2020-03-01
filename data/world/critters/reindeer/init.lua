dirname = path.dirname(__file__)

animations = {
   idle = {
      directory = dirname,
      basename ="idle",
      hotspot = { 23, 21 },
      fps = 20,
   },
}

add_directional_animation(animations, "walk", dirname, "walk", {25, 30}, 30)


world:new_critter_type{
   name = "reindeer",
   descname = _ "Reindeer",
   editor_category = "critters_herbivores",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}
