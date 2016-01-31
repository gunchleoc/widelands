include "tribes/scripting/help/format_help.lua"

-- RST
-- worker_help.lua
-- ---------------

-- Functions used in the ingame worker help windows for formatting the text and pictures.

--  =======================================================
--  ************* Main worker help functions *************
--  =======================================================

function worker_help_producers_string(tribe, worker_description)
   local result = ""
   for i, building_name in ipairs(tribe.buildings) do
      local building = wl.Game():get_building_description(building_name)
      if (building.type_name == "productionsite") then
         local recruits_this = false;
         for j, output in ipairs(building.output_worker_types) do
            if (output.name == worker_description.name) then
               recruits_this = true;
               break;
            end
         end

         if (recruits_this) then
            -- TRANSLATORS: Worker Encyclopedia: A building recruiting a worker
            result = result .. rt(h2(_"Producer"))
            result = result .. dependencies({building, worker_description}, building.descname)

            -- Find out which programs in the building recruit this worker if any
            local producing_programs = {}
            for j, program_name in ipairs(building.production_programs) do
               for worker, amount in pairs(building:recruited_workers(program_name)) do
                  if (worker_description.name == worker) then
                     table.insert(producing_programs, program_name)
                  end
               end
            end

            -- Now collect all workers recruited by the filtered programs
            local recruited_workers_strings = {}
            local recruited_workers_counters = {}
            for j, program_name in ipairs(producing_programs) do
               local recruited_workers_amount = {}
               recruited_workers_counters[program_name] = 0
               for worker, amount in pairs(building:recruited_workers(program_name)) do
                  if (recruited_workers_amount[worker] == nil) then
                     recruited_workers_amount[worker] = 0
                  end
                  recruited_workers_amount[worker] = recruited_workers_amount[worker] + amount
                  recruited_workers_counters[program_name] = recruited_workers_counters[program_name] + amount
               end
               local produced_wares_string = ""
               for ware, amount in pairs(recruited_workers_amount) do
               local ware_descr = wl.Game():get_worker_description(ware)
                  produced_wares_string = produced_wares_string
                     .. help_ware_amount_line(ware_descr, amount)
               end
               recruited_workers_strings[program_name] = produced_wares_string
            end

            -- Now collect the consumed wares for each filtered program and print the program info
            for j, program_name in ipairs(producing_programs) do
               result = result .. help_consumed_wares(building, program_name)
               if (recruited_workers_counters[program_name] > 0) then
                  result = result
                     -- TRANSLATORS: Worker Encyclopedia: Workers recruited by a productionsite
                     .. rt(h3(ngettext("Worker recruited:", "Workers recruited:", recruited_workers_counters[program_name])))
                     .. recruited_workers_strings[program_name]
               end
            end
         end
      end
   end
   return result
end

-- RST
-- .. function worker_help_string(worker_description)
--
--    Displays the worker with a helptext, an image and the tool used
--
--    :arg tribe: The :class:`LuaTribeDescription` for the tribe
--                that we are displaying this help for.
--
--    :arg worker_description: the worker_description from C++.
--    :returns: Help string for the worker
--
function worker_help_string(tribe, worker_description)
   include(worker_description.helptext_script)

   local result = rt(h2(_"Purpose")) ..
      rt("image=" .. worker_description.icon_name, p(worker_helptext()))

   if (worker_description.is_buildable) then
      -- Get the tools for the workers.
      local toolnames = {}
      for j, buildcost in ipairs(worker_description.buildcost) do
         if (buildcost ~= nil and tribe:has_ware(buildcost)) then
            toolnames[#toolnames + 1] = buildcost
         end
      end

      if(#toolnames > 0) then
         result = result .. help_tool_string(tribe, toolnames, 1)
      end
   else
      result = result .. worker_help_producers_string(tribe, worker_description)
   end


   -- TODO(GunChleoc): Add "enhanced from" info in one_tribe branch
   local becomes_description = worker_description.becomes
   if (becomes_description) then

      result = result .. rt(h3(_"Experience levels:"))
      local exp_string = _"%s to %s (%s EP)":format(
            worker_description.descname,
            becomes_description.descname,
            worker_description.needed_experience
         )

      worker_description = becomes_description
      becomes_description = worker_description.becomes
      if(becomes_description) then
         exp_string = exp_string .. "<br>" .. _"%s to %s (%s EP)":format(
               worker_description.descname,
               becomes_description.descname,
               worker_description.needed_experience
            )
      end
      result = result ..  rt("text-align=right", p(exp_string))
   end
   return result
end


return {
   func = function(tribename, worker_description)
      set_textdomain("tribes_encyclopedia")
      local tribe = wl.Game():get_tribe_description(tribename)
      return worker_help_string(tribe, worker_description)
   end
}
