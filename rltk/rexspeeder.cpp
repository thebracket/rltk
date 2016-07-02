#include "rexspeeder.hpp"
#include <zlib.h>
#include <stdexcept>

namespace rltk {

//===========================================================================================================//
//    Safe I/O (where "safe" means "will throw errors")                                                      //
//                                                                                                           //
//   These functions will throw an error message from gzerror, and set errno to the error code.              //
//===========================================================================================================//

inline std::runtime_error make_rexception(gzFile g) {
	/*The exception creation is a bit verbose.*/
	int errnum = 0;
	const char* errstr = gzerror(g, &errnum);
	return std::runtime_error(std::to_string(errnum) + std::string(" ") + std::string(errstr));
}

static void s_gzread(gzFile g, voidp buf, unsigned int len)
{
	if (gzread(g, buf, len) > 0)
		return;

	/*We expect to read past the end of the file after the last layer.*/
	if (gzeof(g))
		return;
	
	throw make_rexception(g);
}

static void s_gzwrite(gzFile g, voidp buf, unsigned int len)
{
	if (gzwrite(g, buf, len) > 0)
		return;

	throw make_rexception(g);
}

static gzFile s_gzopen(const std::string filename, const char* permissions)
{
	gzFile g = gzopen(filename.c_str(), permissions);

	if (g != Z_NULL)
		return g;

	int err = 0;
	const char* errstr = gzerror(g, &err);
	if (err == 0) {
		/*Assume the file simply didn't exist.*/
		std::string s("File " + filename + " does not exist.");
		throw std::runtime_error(s);
	}
	throw std::runtime_error(std::string(errstr));
}


namespace xp {

//===========================================================================================================//
//    Loading an xp file                                                                                     //
//===========================================================================================================//
	rex_sprite::rex_sprite(std::string const & filename)
	{
		typedef void* vp;
		//Number of bytes in a tile. Not equal to sizeof(RexTile) due to padding.
		const int tileLen = 10; 

		gzFile gz;
		try {
			gz = s_gzopen(filename.c_str(), "rb");

			s_gzread(gz, (vp)&version, sizeof(version));
			s_gzread(gz, (vp)&num_layers, sizeof(num_layers));
			s_gzread(gz, (vp)&width, sizeof(width));
			s_gzread(gz, (vp)&height, sizeof(height));

			layers.resize(num_layers);

			for (int i = 0; i < num_layers; i++)
				layers[i] = rex_layer(width, height);

			for (int layer_index = 0; layer_index < num_layers; layer_index++) {
				for (int i = 0; i < width*height; ++i)
					s_gzread(gz, get_tile(layer_index, i), tileLen);

				//The layer and height information is repeated.
				//This is expected to read off the end after the last layer.
				s_gzread(gz, (vp)&width, sizeof(width));
				s_gzread(gz, (vp)&height, sizeof(height));
			}
		}
		catch (...) { throw; }

		gzclose(gz);
	}

//===========================================================================================================//
//    Saving an xp file                                                                                      //
//===========================================================================================================//
	void rex_sprite::save(std::string const & filename)
	{
		typedef void* vp;
		//Number of bytes in a tile. Not equal to sizeof(RexTile) due to padding.
		const int tileLen = 10; 

		try {
			gzFile gz = s_gzopen(filename.c_str(), "wb");

			s_gzwrite(gz, (vp)&version, sizeof(version));
			s_gzwrite(gz, (vp)&num_layers, sizeof(num_layers));

			for (int layer = 0; layer < num_layers; ++layer) {
				s_gzwrite(gz, (vp)&width, sizeof(width));
				s_gzwrite(gz, (vp)&height, sizeof(height));

				for (int i = 0; i < width*height; ++i) 
					//Note: not "sizeof(RexTile)" because of padding.
					s_gzwrite(gz, (vp)get_tile(layer,i), tileLen);
				
			}

			gzflush(gz, Z_FULL_FLUSH);
			gzclose(gz);
		}
		catch (...) { throw; }
	}

//===========================================================================================================//
//    Constructors / Destructors                                                                             //
//===========================================================================================================//
	rex_sprite::rex_sprite(int _version, int _width, int _height, int _num_layers)
		:version(_version), width(_width), height(_height), num_layers(_num_layers)
	{
		//All layers above the first are set transparent.
		for (int l = 1; l < num_layers; l++) {
			for (int i = 0; i < width*height; ++i) {
				rltk::vchar t = transparent_tile();
				set_tile(l, i, t);
			}
		}
	}

//===========================================================================================================//
//    Utility Functions                                                                                      //
//===========================================================================================================//
	void rex_sprite::flatten() {
		if (num_layers == 1)
			return;

		//Paint the last layer onto the second-to-last
		for (int i = 0; i < width*height; ++i) {
			rltk::vchar* overlay = get_tile(num_layers - 1, i);
			if (!is_transparent(overlay)) {
				*get_tile(num_layers - 2, i) = *overlay;
			}
		}

		//Remove the last layer
		--num_layers;

		//Recurse
		flatten();
	}	

}

}