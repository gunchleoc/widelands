dirname = path.dirname(__file__)

animations = {
   idle = {
      directory = dirname,
      basename ="idle",
      hotspot = { 15, 27 },
      fps = 20,
      sound_effect = {
         -- Sound files with numbers starting for 10 are generating silence. Remove when we move the sound triggering to programs
         path = "sound/animals/elk",
      },
   },
}

add_directional_animation(animations, "walk", dirname, "walk", {21, 34}, 25)

world:new_critter_type{
   name = "moose",
   descname = _ "Moose",
   editor_category = "critters_herbivores",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}
