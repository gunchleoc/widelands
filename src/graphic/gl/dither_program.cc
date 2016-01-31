/*
 * Copyright (C) 2006-2015 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "graphic/gl/dither_program.h"

#include "base/wexception.h"
#include "graphic/gl/coordinate_conversion.h"
#include "graphic/gl/fields_to_draw.h"
#include "graphic/gl/utils.h"
#include "graphic/image_io.h"
#include "graphic/texture.h"
#include "io/filesystem/layered_filesystem.h"

namespace  {

const char kDitherVertexShader[] = R"(
#version 120

// Attributes.
attribute float attr_brightness;
attribute vec2 attr_dither_texture_position;
attribute vec2 attr_position;
attribute vec2 attr_texture_offset;
attribute vec2 attr_texture_position;

uniform float u_z_value;

// Output of vertex shader.
varying float var_brightness;
varying vec2 var_dither_texture_position;
varying vec2 var_texture_offset;
varying vec2 var_texture_position;

void main() {
	var_brightness = attr_brightness;
	var_dither_texture_position = attr_dither_texture_position;
	var_texture_offset = attr_texture_offset;
	var_texture_position = attr_texture_position;
	gl_Position = vec4(attr_position, u_z_value, 1.);
}
)";

const char kDitherFragmentShader[] = R"(
#version 120

uniform sampler2D u_dither_texture;
uniform sampler2D u_terrain_texture;
uniform vec2 u_texture_dimensions;

varying float var_brightness;
varying vec2 var_dither_texture_position;
varying vec2 var_texture_position;
varying vec2 var_texture_offset;

// TODO(sirver): This is a hack to make sure we are sampling inside of the
// terrain texture. This is a common problem with OpenGL and texture atlases.
#define MARGIN 1e-2

void main() {
	vec2 texture_fract = clamp(
			fract(var_texture_position),
			vec2(MARGIN, MARGIN),
			vec2(1. - MARGIN, 1. - MARGIN));
	vec4 clr = texture2D(u_terrain_texture, var_texture_offset + u_texture_dimensions * texture_fract);
	gl_FragColor = vec4(clr.rgb * var_brightness,
			1. - texture2D(u_dither_texture, var_dither_texture_position).a);
}
)";

}  // namespace

DitherProgram::DitherProgram() {
	gl_program_.build(kDitherVertexShader, kDitherFragmentShader);

	attr_brightness_ = glGetAttribLocation(gl_program_.object(), "attr_brightness");
	attr_dither_texture_position_ = glGetAttribLocation(gl_program_.object(), "attr_dither_texture_position");
	attr_position_ = glGetAttribLocation(gl_program_.object(), "attr_position");
	attr_texture_offset_ = glGetAttribLocation(gl_program_.object(), "attr_texture_offset");
	attr_texture_position_ = glGetAttribLocation(gl_program_.object(), "attr_texture_position");

	u_dither_texture_ = glGetUniformLocation(gl_program_.object(), "u_dither_texture");
	u_terrain_texture_ = glGetUniformLocation(gl_program_.object(), "u_terrain_texture");
	u_texture_dimensions_ = glGetUniformLocation(gl_program_.object(), "u_texture_dimensions");
	u_z_value_ = glGetUniformLocation(gl_program_.object(), "u_z_value");

	dither_mask_.reset(new Texture(load_image_as_sdl_surface("world/pics/edge.png", g_fs), true));

	Gl::State::instance().bind(GL_TEXTURE0, dither_mask_->blit_data().texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLint>(GL_CLAMP_TO_EDGE));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLint>(GL_CLAMP_TO_EDGE));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(GL_LINEAR));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(GL_LINEAR));
}

DitherProgram::~DitherProgram() {}

void DitherProgram::add_vertex(const FieldsToDraw::Field& field,
                               const TrianglePoint triangle_point,
                               const FloatPoint& texture_offset) {
	vertices_.emplace_back();
	PerVertexData& back = vertices_.back();

	back.gl_x = field.gl_x;
	back.gl_y = field.gl_y;
	back.texture_x = field.texture_x;
	back.texture_y = field.texture_y;
	back.brightness = field.brightness;
	back.texture_offset_x = texture_offset.x;
	back.texture_offset_y = texture_offset.y;

	switch (triangle_point) {
	case TrianglePoint::kTopRight:
		back.dither_texture_x = 1.;
		back.dither_texture_y = 1.;
		break;
	case TrianglePoint::kTopLeft:
		back.dither_texture_x = 0.;
		back.dither_texture_y = 1.;
		break;
	case TrianglePoint::kBottomMiddle:
		back.dither_texture_x = 0.5;
		back.dither_texture_y = 0.;
		break;
	}
}

void DitherProgram::maybe_add_dithering_triangle(
   const uint32_t gametime,
   const DescriptionMaintainer<Widelands::TerrainDescription>& terrains,
   const FieldsToDraw& fields_to_draw,
   const int idx1,
   const int idx2,
   const int idx3,
   const int my_terrain,
   const int other_terrain) {
	if (my_terrain == other_terrain) {
		return;
	}
	const Widelands::TerrainDescription& other_terrain_description =
	   terrains.get(other_terrain);
	if (terrains.get(my_terrain).dither_layer() <
	    other_terrain_description.dither_layer()) {
		const FloatPoint texture_offset =
		   to_gl_texture(other_terrain_description.get_texture(gametime).blit_data()).origin();
		add_vertex(fields_to_draw.at(idx1), TrianglePoint::kTopRight, texture_offset);
		add_vertex(fields_to_draw.at(idx2), TrianglePoint::kTopLeft, texture_offset);
		add_vertex(fields_to_draw.at(idx3), TrianglePoint::kBottomMiddle, texture_offset);
	}
}

void DitherProgram::gl_draw(int gl_texture, float texture_w, float texture_h, const float z_value) {
	glUseProgram(gl_program_.object());

	auto& gl_state = Gl::State::instance();
	gl_state.enable_vertex_attrib_array({attr_brightness_,
	                                   attr_dither_texture_position_,
	                                   attr_position_,
	                                   attr_texture_offset_,
	                                   attr_texture_position_});

	gl_array_buffer_.bind();
	gl_array_buffer_.update(vertices_);

	Gl::vertex_attrib_pointer(
	   attr_brightness_, 1, sizeof(PerVertexData), offsetof(PerVertexData, brightness));
	Gl::vertex_attrib_pointer(attr_dither_texture_position_,
	                       2,
	                       sizeof(PerVertexData),
	                       offsetof(PerVertexData, dither_texture_x));
	Gl::vertex_attrib_pointer(attr_position_, 2, sizeof(PerVertexData), offsetof(PerVertexData, gl_x));
	Gl::vertex_attrib_pointer(
	   attr_texture_offset_, 2, sizeof(PerVertexData), offsetof(PerVertexData, texture_offset_x));
	Gl::vertex_attrib_pointer(
	   attr_texture_position_, 2, sizeof(PerVertexData), offsetof(PerVertexData, texture_x));

	gl_state.bind(GL_TEXTURE0, dither_mask_->blit_data().texture_id);
	gl_state.bind(GL_TEXTURE1, gl_texture);

	glUniform1f(u_z_value_, z_value);
	glUniform1i(u_dither_texture_, 0);
	glUniform1i(u_terrain_texture_, 1);
	glUniform2f(u_texture_dimensions_, texture_w, texture_h);

	glDrawArrays(GL_TRIANGLES, 0, vertices_.size());
}

void DitherProgram::draw(const uint32_t gametime,
                         const DescriptionMaintainer<Widelands::TerrainDescription>& terrains,
                         const FieldsToDraw& fields_to_draw,
                         const float z_value) {
	// This method expects that all terrains have the same dimensions and that
	// all are packed into the same texture atlas, i.e. all are in the same GL
	// texture. It does not check for this invariance for speeds sake.

	vertices_.clear();
	vertices_.reserve(fields_to_draw.size() * 3);

	for (size_t current_index = 0; current_index < fields_to_draw.size(); ++current_index) {
		const FieldsToDraw::Field& field = fields_to_draw.at(current_index);

		// The bottom right neighbor fields_to_draw is needed for both triangles
		// associated with this field. If it is not in fields_to_draw, there is no need to
		// draw any triangles.
		const int brn_index = fields_to_draw.calculate_index(field.fx + (field.fy & 1), field.fy + 1);
		if (brn_index == -1) {
			continue;
		}

		// Dithering triangles for Down triangle.
		const int bln_index =
		   fields_to_draw.calculate_index(field.fx + (field.fy & 1) - 1, field.fy + 1);
		if (bln_index != -1) {
			maybe_add_dithering_triangle(gametime, terrains, fields_to_draw,
			   brn_index, current_index, bln_index, field.ter_d, field.ter_r);

			const int terrain_dd = fields_to_draw.at(bln_index).ter_r;
			maybe_add_dithering_triangle(gametime, terrains, fields_to_draw,
			   bln_index, brn_index, current_index, field.ter_d, terrain_dd);

			const int ln_index = fields_to_draw.calculate_index(field.fx - 1, field.fy);
			if (ln_index != -1) {
				const int terrain_l = fields_to_draw.at(ln_index).ter_r;
				maybe_add_dithering_triangle(gametime, terrains, fields_to_draw,
				   current_index, bln_index, brn_index, field.ter_d, terrain_l);
			}
		}

		// Dithering for right triangle.
		const int rn_index = fields_to_draw.calculate_index(field.fx + 1, field.fy);
		if (rn_index != -1) {
			maybe_add_dithering_triangle(gametime, terrains, fields_to_draw,
			   current_index, brn_index, rn_index, field.ter_r, field.ter_d);
			int terrain_rr = fields_to_draw.at(rn_index).ter_d;
			maybe_add_dithering_triangle(gametime, terrains, fields_to_draw,
					brn_index, rn_index, current_index, field.ter_r, terrain_rr);

			const int trn_index =
				fields_to_draw.calculate_index(field.fx + (field.fy & 1), field.fy - 1);
			if (trn_index != -1) {
				const int terrain_u = fields_to_draw.at(trn_index).ter_d;
				maybe_add_dithering_triangle(gametime, terrains, fields_to_draw,
				   rn_index, current_index, brn_index, field.ter_r, terrain_u);
			}
		}
	}

	const BlitData& blit_data = terrains.get(0).get_texture(0).blit_data();
	const FloatRect texture_coordinates = to_gl_texture(blit_data);
	gl_draw(blit_data.texture_id,
	        texture_coordinates.w,
	        texture_coordinates.h,
	        z_value);
}
