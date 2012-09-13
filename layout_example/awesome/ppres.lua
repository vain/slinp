local tonumber = tonumber
local awful = awful
local table = table

module("vain.layout.ppres")
name = "ppres"

function arrange(p)
    local wa = p.workarea
    local cls = p.clients
    local i

    local main_row_relative = 0.5
    local slaves = {}

    for i = 1,#cls
    do
        c = cls[i]
        if c.class == "Showpdf"
        then
            if c.instance == "projector"
            then
                awful.client.floating.set(c, true)
                c.ontop = true
            else
                local min, max, this = c.instance:match("([^_]+)_([^_]+)_([^_]+)")
                if min == nil or max == nil or this == nil
                then
                    table.insert(slaves, c)
                else
                    min = tonumber(min)
                    max = tonumber(max)
                    this = tonumber(this)

                    local slots = max - min + 1
                    local this_slot = this - min

                    local x_offset_relative = this_slot / slots

                    local g = {}
                    g.x = wa.x + x_offset_relative * wa.width
                    g.y = wa.y
                    g.width = 1 / slots * wa.width
                    g.height = main_row_relative * wa.height
                    c:geometry(g)
                end
            end
        else
            table.insert(slaves, c)
        end
    end

    for i = 1,#slaves
    do
        local g = {}
        g.x = wa.x + ((i - 1) / #slaves) * wa.width
        g.y = wa.y + main_row_relative * wa.height
        g.width = 1 / #slaves * wa.width
        g.height = wa.height - main_row_relative * wa.height
        slaves[i]:geometry(g)
    end
end

-- vim: set et :
