#include "rexspeeder.hpp"
#include <zlib.h>

//===========================================================================================================//
//    Safe I/O (where "safe" means "will throw errors")                                                      //
//                                                                                                           //
//   These functions will throw an error message from gzerror, and set errno to the error code.              //
//===========================================================================================================//

static xp::Rexception makeRexception(gzFile g) {
	/*The exception creation is a bit verbose.*/
	int errnum = 0;
	const char* errstr = gzerror(g, &errnum);
	xp::Rexception e(errstr,errnum);
	return e;
}

static void s_gzread(gzFile g, voidp buf, unsigned int len)
{
	if (gzread(g, buf, len) > 0)
		return;

	/*We expect to read past the end of the file after the last layer.*/
	if (gzeof(g))
		return;
	
	throw makeRexception(g);
}

static void s_gzwrite(gzFile g, voidp buf, unsigned int len)
{
	if (gzwrite(g, buf, len) > 0)
		return;

	throw makeRexception(g);
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
		xp::Rexception e(s, xp::ERR_FILE_DOES_NOT_EXIST);
		throw e;
	}
	xp::Rexception e(errstr, err);
	throw e;
}


namespace xp {
	//General method for ensuring an image has the correct number of layers.
	static void enforceValidLayerCount(xp::RexImage& img) {
		if (img.getNumLayers() < 1 || img.getNumLayers() > 4)
			throw xp::Rexception("Invalid number of layers.", xp::ERR_INVALID_NUMBER_OF_LAYERS);
	}

//===========================================================================================================//
//    Loading an xp file                                                                                     //
//===========================================================================================================//
	RexImage::RexImage(std::string const & filename)
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

			enforceValidLayerCount(*this);

			for (int i = 0; i < num_layers; i++)
				layers[i] = RexLayer(width, height);

			for (int layer_index = 0; layer_index < num_layers; layer_index++) {
				for (int i = 0; i < width*height; ++i)
					s_gzread(gz, getTile(layer_index, i), tileLen);

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
	void RexImage::save(std::string const & filename)
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
					s_gzwrite(gz, (vp)getTile(layer,i), tileLen);
				
			}

			gzflush(gz, Z_FULL_FLUSH);
			gzclose(gz);
		}
		catch (...) { throw; }
	}

//===========================================================================================================//
//    Constructors / Destructors                                                                             //
//===========================================================================================================//
	RexImage::RexImage(int _version, int _width, int _height, int _num_layers)
		:version(_version), width(_width), height(_height), num_layers(_num_layers)
	{
		enforceValidLayerCount(*this);

		//All layers above the first are set transparent.
		for (int l = 1; l < num_layers; l++) {
			for (int i = 0; i < width*height; ++i) {
				RexTile t = transparentTile();
				setTile(l, i, t);
			}
		}
	}

//===========================================================================================================//
//    Utility Functions                                                                                      //
//===========================================================================================================//
	void RexImage::flatten() {
		if (num_layers == 1)
			return;

		//Paint the last layer onto the second-to-last
		for (int i = 0; i < width*height; ++i) {
			RexTile* overlay = getTile(num_layers - 1, i);
			if (!isTransparent(overlay)) {
				*getTile(num_layers - 2, i) = *overlay;
			}
		}

		//Remove the last layer
		--num_layers;

		//Recurse
		flatten();
	}


	bool isTransparent(RexTile * tile)
	{
		//This might be faster than comparing with transparentTile(), despite it being a constexpr
		return (tile->back_red == 255 && tile->back_green == 0 && tile->back_blue == 255);
	}	

//===========================================================================================================//
//    RexLayer constructor/destructor                                                                        //
//===========================================================================================================//

	RexLayer::RexLayer(int width, int height) 
	{
		tiles.resize(width * height);
	} 

	RexLayer::~RexLayer()
	{
		tiles.clear();
	}
}