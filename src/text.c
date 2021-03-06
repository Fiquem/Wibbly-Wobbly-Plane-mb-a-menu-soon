#include "text.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "maths_funcs.h"

GLuint text_vao, text_point_vbo, text_tex_vbo;

Font load_font (const char* font_img, const char* font_img_s, const char* font_meta){
	Font f;
	f.size = DEFAULT_FONT_SIZE * 1.5;

	// load font tex
	int x, y, n;
    unsigned char* image = stbi_load(font_img, &x, &y, &n, 0);
    unsigned char* image_s = stbi_load(font_img_s, &x, &y, &n, 0);

	glActiveTexture(GL_TEXTURE0);
	glGenTextures (1, &f.texture);
	glActiveTexture (f.texture);
	glBindTexture (GL_TEXTURE_2D, f.texture);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glActiveTexture(GL_TEXTURE0);
	glGenTextures (1, &f.texture_selected);
	glActiveTexture (f.texture_selected);
	glBindTexture (GL_TEXTURE_2D, f.texture_selected);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_s);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// load font shader
	create_program_from_files ("text.vert", "text.frag", &f.shader);
	glUseProgram (f.shader.program);
	glUniformMatrix4fv (f.shader.P_loc, 1, GL_FALSE, ortho(0.0, INIT_WIN_WIDTH, 0.0, INIT_WIN_HEIGHT, 0.9, 1.1).m);
	glUniform3f (f.shader.colour_loc, 0.047, 0.067, 0.224);

	// jus copied I@ll do better later so tired now
	glGenBuffers(1, &text_point_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, text_point_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 12, NULL, GL_STATIC_DRAW);
	glGenBuffers(1, &text_tex_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, text_tex_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 12, NULL, GL_STATIC_DRAW);

	glGenVertexArrays(1, &text_vao);
	glBindVertexArray(text_vao);
	glEnableVertexAttribArray (POINT);
	glEnableVertexAttribArray (TEX_COORD);
	glBindBuffer (GL_ARRAY_BUFFER, text_point_vbo);
	glVertexAttribPointer (POINT, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer (GL_ARRAY_BUFFER, text_tex_vbo);
	glVertexAttribPointer (TEX_COORD, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	// load character info from meta
	// NO META FOR THE MOMENT
	int rows = 16;
	int cols = 16;
	int char_size_px = 1024/16;
	f.chars = (Character*)malloc(sizeof(Character)*256);
	for (int i = 0; i < rows; i++)
		for (int j = 0; j < cols; j++){
			f.chars[(i*cols) + j].xpos = j;
			f.chars[(i*cols) + j].ypos = rows - i;
			f.chars[(i*cols) + j].width = char_size_px;
			f.chars[(i*cols) + j].height = char_size_px;
			f.chars[(i*cols) + j].buffer = 0;
		}

	return f;
}

Text set_text (Font f, const char* s, int w, int h, int x, int y){
	Text t;
	t.font = f;
	t.text = strcat((char*)s, "\0");
	t.selected = false;
	t.width = w;
	t.height = h;
	t.xpos = x;
	t.ypos = y;
	
	t.length = 0;
	int i = 0;
	while(t.text[i] != '\0'){
		// check if regular char or outside char range
		if (t.text[i] < ' ' || t.text[i] > '~'+1) i+=2;
		else i++;
		t.length++;
	}

	return t;
}

void set_text_pos(Text* t, float x, float y){
	t->xpos = x;
	t->ypos = y;
}

void set_text_pos(Text* t, Text_Positions pos){
	switch (pos) {
		case TOP: {
			t->ypos = t->height - t->font.size;
			break;
		}
		case BOTTOM: {
			t->ypos = 0.0;
			break;
		}
		case LEFT: {
			t->xpos = 0.0;
			break;
		}
		case RIGHT: {
			t->xpos = t->width - (t->length * t->font.size);
			break;
		}
		case CENTRE: {
			t->ypos = t->height/2.0;
			t->xpos = (t->width - (t->length * t->font.size))/2.0;
			break;
		}
	}
}

void draw_text (Text t){
	glUseProgram (t.font.shader.program);
	glEnable (GL_BLEND);
    glDisable (GL_DEPTH_TEST);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(text_vao);

    Character c;
	int i = 0;
	float x = t.xpos;
	float y = t.ypos;
	float xpos, ypos, w, h;
	unsigned char char_shifted;
	vec3 text_colour = vec3(0.047, 0.067, 0.224);
	vec3 text_selected = vec3(1.0, 0.271, 0.755);
	vec3 text_unselected = vec3(1.0, 1.0, 1.0);
	vec3 text_highlight = vec3(0.055, 0.871, 0.055);
	while(t.text[i] != '\0'){

		// check if regular char or outside char range
		if (t.text[i] < ' ' || t.text[i] > '~'+1){
			char_shifted = t.text[i];
			char_shifted <<= 8;
			i++;
			char_shifted += t.text[i];
			c = t.font.chars[char_shifted - ' '];
		} else
			c = t.font.chars[t.text[i] - ' '];

        xpos = c.xpos * 64;
        ypos = c.ypos * 64;

        w = t.font.size;
        h = t.font.size;

        // Update VBO for each character
	    GLfloat vertices[12] = {
	        x,     y,
	        x + w, y,
	        x,     y + h,

	        x,     y + h,
	        x + w, y,
	        x + w, y + h
	    };
	    GLfloat tex_coords[12] = {
	        xpos, ypos - c.height,
	        xpos + c.width, ypos - c.height,
	        xpos, ypos,

	        xpos, ypos,
	        xpos + c.width, ypos - c.height,
	        xpos + c.width, ypos
	    };

	    // resize
	    for (int j = 0; j < 12; j++)
	    	tex_coords[j] /= 1024.0;

        glBindBuffer(GL_ARRAY_BUFFER, text_point_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 12, vertices, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, text_tex_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 12, tex_coords, GL_DYNAMIC_DRAW);

        if (t.selected) text_highlight = text_selected;
        else text_highlight = text_unselected;

        // Draw letter
    	glBindTexture(GL_TEXTURE_2D, t.font.texture_selected);
		glUniform3f(t.font.shader.colour_loc, text_highlight.v[0], text_highlight.v[1], text_highlight.v[2]);
 		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindTexture(GL_TEXTURE_2D, t.font.texture);
		glUniform3f(t.font.shader.colour_loc, text_colour.v[0], text_colour.v[1], text_colour.v[2]);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        if (x + (2*t.font.size) <= t.width) x += t.font.size;
        else {
        	x = t.xpos;
        	y -= t.font.size;
        }

		i++;
	}
	
    glEnable (GL_DEPTH_TEST);
	glDisable (GL_BLEND);
}