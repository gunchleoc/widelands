-- RST
-- terrain_help.lua
-- ----------------
--
-- This script returns a formatted entry for the terrain help in the editor.
-- Pass the internal terrain name to the coroutine to select the terrain type.

include "scripting/editor/format_editor.lua"

return {
   func = function(terrain_name)
      set_textdomain("widelands_editor")
      local world = wl.World();
      local terrain = wl.Editor():get_terrain_description(terrain_name)

      local result = picture_li(terrain.representative_image, "")

      -- Resources
      local valid_resources = terrain.valid_resources
      if (#valid_resources > 0) then
         result = result .. spacer() .. rt(h2(_"Resources"))
         if (#valid_resources > 0) then
            -- TRANSLATORS: A header in the editor help
            result = result .. rt(h3(ngettext(
               "Valid Resource:", "Valid Resources:", #valid_resources)))
            for count, resource in pairs(valid_resources) do
               result = result .. picture_li(
                  resource.representative_image, resource.descname)
            end
         end

         local default_resource = terrain.default_resource
         if (default_resource ~= nil) then
            result = result .. text_line(_"Default:",
               -- TRANSLATORS: e.g. "5x Water"
               _"%1%x %2%":bformat(terrain.default_resource_amount, default_resource.descname))
         end
      end

      -- Trees
      local tree_list = {}
      for i, immovable in ipairs(world.immovable_descriptions) do
         if (immovable:has_attribute("tree")) then
            local probability = immovable:probability_to_grow(terrain)
            if (probability > 0.01) then
               -- sort the trees by percentage
               i = 1
               while (tree_list[i] and (tree_list[i].probability > probability)) do
                  i = i + 1
               end

               for j = #tree_list, i, -1 do
                  tree_list[j+1] = tree_list[j]
               end
               tree_list[i] = {tree = immovable, probability = probability}
            end
         end
      end

      local tree_string = ""
      for k,v in ipairs(tree_list) do
         tree_string = tree_string .. picture_li(v.tree.representative_image,
            v.tree.species .. ("<br>%2.1f%%"):bformat(100 * v.probability)) .. spacer()
      end

      -- TRANSLATORS: A header in the editor help
      result = result .. spacer() .. rt(h2(_"Probability of trees growing")) .. spacer()

      if (tree_string ~="") then
         result = result .. tree_string
      else
         result = result .. rt(p(_"No trees will grow here."))
      end
      return {
         title = terrain.descname,
         text = result
      }
   end
}
