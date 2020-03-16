#include <eepp/graphics/circledrawable.hpp>
#include <eepp/graphics/drawable.hpp>
#include <eepp/graphics/drawablesearcher.hpp>
#include <eepp/graphics/rectangledrawable.hpp>
#include <eepp/graphics/triangledrawable.hpp>
#include <eepp/ui/css/drawableimageparser.hpp>
#include <eepp/ui/uinode.hpp>

namespace EE { namespace UI { namespace CSS {

DrawableImageParser::DrawableImageParser() {
	registerBaseParsers();
}

bool DrawableImageParser::exists( const std::string& name ) const {
	return mFuncs.find( name ) != mFuncs.end();
}

Drawable* DrawableImageParser::createDrawable( const std::string& value, const Sizef& size,
											   bool& ownIt, UINode* node ) {
	FunctionString functionType = FunctionString::parse( value );
	Drawable* res = NULL;
	ownIt = false;

	if ( !functionType.isEmpty() ) {
		if ( exists( functionType.getName() ) ) {
			return mFuncs[ functionType.getName() ]( functionType, size, ownIt, node );
		}
	} else if ( NULL != ( res = DrawableSearcher::searchByName( value ) ) ) {
		return res;
	}

	return res;
}

void DrawableImageParser::addParser( const std::string& name,
									 const DrawableImageParserFunc& func ) {
	if ( exists( name ) ) {
		eePRINTL( "DrawableImageParser::addParser: image parser function \"%s\" already exists.",
				  name.c_str() );
		return;
	}

	mFuncs[name] = func;
}

void DrawableImageParser::registerBaseParsers() {
	mFuncs["linear-gradient"] = []( const FunctionString& functionType, const Sizef& size,
									bool& ownIt, UINode* node ) -> Drawable* {
		if ( functionType.getParameters().size() < 2 ) {
			return NULL;
		}

		RectangleDrawable* drawable = RectangleDrawable::New();
		RectColors rectColors;
		const std::vector<std::string>& params( functionType.getParameters() );

		if ( Color::isColorString( params.at( 0 ) ) && params.size() >= 2 ) {
			rectColors.TopLeft = rectColors.TopRight = Color::fromString( params.at( 0 ) );
			rectColors.BottomLeft = rectColors.BottomRight = Color::fromString( params.at( 1 ) );
		} else if ( params.size() >= 3 ) {
			std::string direction = params.at( 0 );
			String::toLowerInPlace( direction );

			if ( direction == "to bottom" ) {
				rectColors.TopLeft = rectColors.TopRight = Color::fromString( params.at( 1 ) );
				rectColors.BottomLeft = rectColors.BottomRight =
					Color::fromString( params.at( 2 ) );
			} else if ( direction == "to left" ) {
				rectColors.TopLeft = rectColors.BottomLeft = Color::fromString( params.at( 2 ) );
				rectColors.TopRight = rectColors.BottomRight = Color::fromString( params.at( 1 ) );
			} else if ( direction == "to right" ) {
				rectColors.TopLeft = rectColors.BottomLeft = Color::fromString( params.at( 1 ) );
				rectColors.TopRight = rectColors.BottomRight = Color::fromString( params.at( 2 ) );
			} else if ( direction == "to top" ) {
				rectColors.TopLeft = rectColors.TopRight = Color::fromString( params.at( 2 ) );
				rectColors.BottomLeft = rectColors.BottomRight =
					Color::fromString( params.at( 1 ) );
			} else {
				rectColors.TopLeft = rectColors.TopRight = Color::fromString( params.at( 1 ) );
				rectColors.BottomLeft = rectColors.BottomRight =
					Color::fromString( params.at( 2 ) );
			}
		} else {
			node->setBackgroundColor( Color::fromString( params.at( 0 ) ) );
			return NULL;
		}

		drawable->setRectColors( rectColors );
		ownIt = true;
		return drawable;
	};

	mFuncs["circle"] = []( const FunctionString& functionType, const Sizef& size, bool& ownIt,
						   UINode* node ) -> Drawable* {
		if ( functionType.getParameters().size() < 1 ) {
			return NULL;
		}

		CircleDrawable* drawable = CircleDrawable::New();

		const std::vector<std::string>& params( functionType.getParameters() );

		CSS::StyleSheetLength length( params[0] );
		drawable->setRadius( node->convertLength( length, size.getWidth() / 2.f ) );

		if ( params.size() >= 2 ) {
			drawable->setColor( Color::fromString( params[1] ) );
		}

		if ( params.size() >= 3 ) {
			std::string fillMode( String::toLower( params[2] ) );
			if ( fillMode == "line" || fillMode == "solid" || fillMode == "fill" ) {
				drawable->setFillMode( fillMode == "line" ? DRAW_LINE : DRAW_FILL );
			}
		}

		drawable->setOffset( drawable->getSize() / 2.f );
		ownIt = true;
		return drawable;
	};

	mFuncs["rectangle"] = []( const FunctionString& functionType, const Sizef& size, bool& ownIt,
							  UINode* node ) -> Drawable* {
		if ( functionType.getParameters().size() < 1 ) {
			return NULL;
		}

		RectangleDrawable* drawable = RectangleDrawable::New();
		RectColors rectColors;
		std::vector<Color> colors;

		const std::vector<std::string>& params( functionType.getParameters() );

		for ( size_t i = 0; i < params.size(); i++ ) {
			std::string param( String::toLower( params[i] ) );

			if ( param == "solid" || param == "fill" ) {
				drawable->setFillMode( DRAW_FILL );
			} else if ( String::startsWith( param, "line" ) ) {
				drawable->setFillMode( DRAW_LINE );

				std::vector<std::string> parts( String::split( param, ' ' ) );

				if ( parts.size() >= 2 ) {
					CSS::StyleSheetLength length( parts[1] );
					drawable->setLineWidth( node->convertLength( length, size.getWidth() ) );
				}
			} else if ( param.find( "º" ) != std::string::npos ) {
				String::replaceAll( param, "º", "" );
				Float floatVal;
				if ( String::fromString( floatVal, param ) ) {
					drawable->setRotation( floatVal );
				}
			} else if ( Color::isColorString( param ) ) {
				colors.push_back( Color::fromString( param ) );
			} else {
				int intVal = 0;

				if ( String::fromString( intVal, param ) ) {
					drawable->setCorners( intVal );
				}
			}
		}

		if ( colors.size() > 0 ) {
			while ( colors.size() < 4 ) {
				colors.push_back( colors[colors.size() - 1] );
			};

			rectColors.TopLeft = colors[0];
			rectColors.BottomLeft = colors[1];
			rectColors.BottomRight = colors[2];
			rectColors.TopRight = colors[3];
			drawable->setRectColors( rectColors );
			ownIt = true;
			return drawable;
		} else {
			eeSAFE_DELETE( drawable );
		}

		return drawable;
	};

	mFuncs["triangle"] = []( const FunctionString& functionType, const Sizef& size, bool& ownIt,
							 UINode* node ) -> Drawable* {
		if ( functionType.getParameters().size() < 2 ) {
			return NULL;
		}

		TriangleDrawable* drawable = TriangleDrawable::New();
		std::vector<Color> colors;
		std::vector<Vector2f> vertices;

		const std::vector<std::string>& params( functionType.getParameters() );

		for ( size_t i = 0; i < params.size(); i++ ) {
			std::string param( String::toLower( params[i] ) );

			if ( Color::isColorString( param ) ) {
				colors.push_back( Color::fromString( param ) );
			} else {
				std::vector<std::string> vertex( String::split( param, ',' ) );

				if ( vertex.size() == 3 ) {
					for ( size_t v = 0; v < vertex.size(); v++ ) {
						vertex[v] = String::trim( vertex[v] );
						std::vector<std::string> coords( String::split( vertex[v], ' ' ) );

						if ( coords.size() == 2 ) {
							CSS::StyleSheetLength posX( coords[0] );
							CSS::StyleSheetLength posY( coords[1] );
							vertices.push_back(
								Vector2f( node->convertLength( posX, size.getWidth() ),
										  node->convertLength( posY, size.getHeight() ) ) );
						}
					}
				}
			}
		}

		if ( vertices.size() == 3 && !colors.empty() ) {
			Triangle2f triangle;

			for ( size_t i = 0; i < 3; i++ ) {
				triangle.V[i] = vertices[i];
			}

			if ( colors.size() == 3 ) {
				drawable->setTriangleColors( colors[0], colors[1], colors[2] );
			} else {
				drawable->setColor( colors[0] );
			}

			drawable->setTriangle( triangle );
			ownIt = true;
			return drawable;
		} else {
			eeSAFE_DELETE( drawable );
		}

		return drawable;
	};

	mFuncs["url"] = []( const FunctionString& functionType, const Sizef& size, bool& ownIt,
						UINode* node ) -> Drawable* {
		if ( functionType.getParameters().size() < 1 ) {
			return NULL;
		}

		return DrawableSearcher::searchByName( functionType.getParameters().at( 0 ) );
	};
}

}}} // namespace EE::UI::CSS
