dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "idle_??",
      directory = dirname,
      hotspot = { 18, 23 },
      fps = 10
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {19, 33}, 10)
add_worker_animations(animations, "walkload", dirname, "walk", {19, 33}, 10) -- TODO(GunChleoc): Make animation


tribes:new_carrier_type {
   msgctxt = "atlanteans_worker",
   name = "atlanteans_horse",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("atlanteans_worker", "Horse"),
   directory = dirname,
   icon = dirname .. "menu.png",
   vision_range = 2,

   default_target_quantity = 10,
   ware_hotspot = {-2, 12},

   animations = animations,
}
