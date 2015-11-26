dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "idle_??",
      directory = dirname,
      hotspot = { 8, 22 }
   },
   freeing = {
      template = "freeing_??",
      directory = dirname,
      hotspot = { 10, 19 },
      fps = 10
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {11, 23}, 20)

tribes:new_worker_type {
   msgctxt = "atlanteans_worker",
   name = "atlanteans_fishbreeder",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Fish Breeder"),
   directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      atlanteans_carrier = 1,
      buckets = 1
   },

   programs = {
      breed = {
         "findspace size:any radius:7 breed resource:fish",
         "walk coords",
         "animation freeing 3000", -- Play a freeing animation
         "breed fish 1",
         "return"
      }
   },

   animations = animations,
}
