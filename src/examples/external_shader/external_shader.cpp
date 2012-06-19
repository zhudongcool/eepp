#include <eepp/ee.hpp>

/// This example is based on the WebGL demo from http://minimal.be/lab/fluGL/
Uint32 ParticlesNum	= 30000;
cWindow * win = NULL;
cShaderProgram * ShaderProgram = NULL;
bool ShadersSupported = false;
eeFloat tw;
eeFloat th;
eeFloat aspectRatio;

void videoResize() {
	/// Video Resize event will re-setup the 2D projection and states, so we must rebuild them.
	aspectRatio	= (eeFloat)win->GetWidth()	/ (eeFloat)win->GetHeight();
	tw			= (eeFloat)win->GetWidth()	/ 2;
	th			= (eeFloat)win->GetHeight()	/ 2;

	eeFloat fieldOfView	= 30.0;
	eeFloat nearPlane	= 1.0;
	eeFloat farPlane	= 10000.0;
	eeFloat top			= nearPlane * eetan(fieldOfView * EE_PI / 360.0);
	eeFloat bottom		= -top;
	eeFloat right		= top * aspectRatio;
	eeFloat left		= -right;

	eeFloat a = (right + left) / (right - left);
	eeFloat b = (top + bottom) / (top - bottom);
	eeFloat c = (farPlane + nearPlane) / (farPlane - nearPlane);
	eeFloat d = (2 * farPlane * nearPlane) / (farPlane - nearPlane);
	eeFloat x = (2 * nearPlane) / (right - left);
	eeFloat y = (2 * nearPlane) / (top - bottom);
	GLfloat perspectiveMatrix[16] = {
		x, 0, a, 0,
		0, y, b, 0,
		0, 0, c, d,
		0, 0, -1, 0
	};

	/// Load the our default projection
	GLi->MatrixMode( GL_PROJECTION );
	GLi->LoadMatrixf( perspectiveMatrix );
	GLi->MatrixMode( GL_MODELVIEW );

	/// eepp enables some this client states by default, and textures by default
	GLi->Disable( GL_TEXTURE_2D );
	GLi->DisableClientState( GL_TEXTURE_COORD_ARRAY );

	/// GL_VERTEX_ARRAY and GL_COLOR_ARRAY are needed, so we keep them enabled
	GLi->EnableClientState( GL_VERTEX_ARRAY );
	GLi->EnableClientState( GL_COLOR_ARRAY );

	/// Reset the default blend func ( by default eepp use ALPHA_NORMAL )
	cTextureFactory::instance()->SetPreBlendFunc( ALPHA_BLENDONE );

	/// Set the line width
	cGlobalBatchRenderer::instance()->SetLineWidth( 2 );

	if ( ShadersSupported ) {
		/// Rebind the Shader
		ShaderProgram->Bind();

		/// If you want to use the programmable-pipeline renderer you'll need to set up the projection and modelview matrix manually.
		/// Or if you want to use another name to the projection matrix or the modelview matrix ( eepp programmable-pipeline use
		/// dgl_ProjectionMatrix and dgl_ModelViewMatrix by default.
		if ( GLv_2 == GLi->Version() ) {
			ShaderProgram->SetUniformMatrix( "dgl_ProjectionMatrix", perspectiveMatrix );

			/// Get the identity matrix and set it to the modelview matrix
			GLfloat modelMatrix[16];
			GLi->LoadIdentity();
			GLi->GetCurrentMatrix( GL_MODELVIEW_MATRIX, modelMatrix );

			ShaderProgram->SetUniformMatrix( "dgl_ModelViewMatrix", modelMatrix );
		}
	}
}

EE_MAIN_FUNC int main (int argc, char * argv [])
{
	win = cEngine::instance()->CreateWindow( WindowSettings( 960, 640, 32, WindowStyle::Default, "", "eepp - External Shaders" ), ContextSettings( true ) );

	if ( win->Created() )
	{
		/// This will work without shaders too
		ShadersSupported = GLi->ShadersSupported();

		cInput * imp = win->GetInput();

		/// We real don't need shaders for this, but the purpose of the example is to show how to work with external shaders
		if ( ShadersSupported ) {
			/// Disable the automatic shader conversion from fixed-pipeline to programmable-pipeline
			cShader::Ensure = false;

			std::string fs( "#ifdef GL_ES\n\
				precision highp float;\n\
				#endif\n\
				varying	vec4 dgl_Color;\n\
				void main() { gl_FragColor = dgl_Color; }" );

			std::string vs( "#ifdef GL_ES\n\
				precision highp float;\n\
				#endif\n\
				attribute vec3 dgl_Vertex;\n\
				attribute vec4 dgl_FrontColor;\n\
				varying	vec4 dgl_Color;\n\
				uniform mat4 dgl_ProjectionMatrix;\n\
				uniform mat4 dgl_ModelViewMatrix;\n\
				void main()	{\n\
					dgl_Color	= dgl_FrontColor;\n\
					gl_Position = dgl_ProjectionMatrix * dgl_ModelViewMatrix * vec4(dgl_Vertex, 1.0);\n\
				}");

			/// Since fixed-pipeline OpenGL use gl_FrontColor for glColorPointer, we need to replace the color attribute
			/// This is all to show how it works, in a real world scenario, you will choose to work fixed-pipeline or programmable-pipeline.
			if ( GLi->Version() == GLv_2 ) {
				ReplaceSubStr( fs, "gl_FragColor = dgl_Color", "gl_FragColor = gl_FrontColor" );
			}

			/// Create the new shader program
			ShaderProgram = eeNew( cShaderProgram, ( vs.c_str(), vs.size(), fs.c_str(), fs.size() ) );
		}

		/// Set the projection
		videoResize();

		/// Push a window resize callback the reset the projection when needed
		win->PushResizeCallback( cb::Make0( &videoResize ) );

		Uint32 i;
		eeVector3ff * vertices		= eeNewArray( eeVector3ff, ParticlesNum );
		eeVector3ff * velocities	= eeNewArray( eeVector3ff, ParticlesNum );
		eeColorAf * colors			= eeNewArray( eeColorAf, ParticlesNum );

		for (i = 0; i < ParticlesNum; i++ )
		{
			vertices[i]		= eeVector3ff( 0, 0, 1.83 );
			velocities[i]	= eeVector3ff( (eeRandf() * 2 - 1)*.05, (eeRandf() * 2 - 1)*.05, .93 + eeRandf()*.02 );
			colors[i]		= eeColorAf( eeRandf() * 0.5, 0.1, 0.8, 0.5 );
		}

		while ( win->Running() )
		{
			imp->Update();

			if ( imp->IsKeyDown( KEY_ESCAPE ) )
			{
				win->Close();
			}

			eeFloat p;
			eeVector2f mf	= imp->GetMousePosf();
			eeFloat tratio	= tw / th;
			eeFloat touchX	= ( mf.x / tw - 1 ) * tratio;
			eeFloat touchY	= -( mf.y / th - 1 );
			bool touch		= imp->MouseLeftPressed();

			for( i = 0; i < ParticlesNum; i+=2 )
			{
				// copy old positions
				vertices[i].x = vertices[i+1].x;
				vertices[i].y = vertices[i+1].y;

				// inertia
				velocities[i].x *= velocities[i].z;
				velocities[i].y *= velocities[i].z;

				// horizontal
				p = vertices[i+1].x;
				p += velocities[i].x;

				if ( p < -aspectRatio ) {
					p = -aspectRatio;
					velocities[i].x = eeabs(velocities[i].x);
				} else if ( p > aspectRatio ) {
					p = aspectRatio;
					velocities[i].x = -eeabs(velocities[i].x);
				}
				vertices[i+1].x = p;

				// vertical
				p = vertices[i+1].y;
				p += velocities[i].y;
				if ( p < -aspectRatio ) {
					p = -aspectRatio;
					velocities[i].y = eeabs(velocities[i].y);
				} else if ( p > aspectRatio ) {
					p = aspectRatio;
					velocities[i].y = -eeabs(velocities[i].y);

				}
				vertices[i+1].y = p;

				if ( touch ) {
					eeFloat dx	= touchX - vertices[i].x;
					eeFloat dy	= touchY - vertices[i].y;
					eeFloat d	= eesqrt( dx * dx + dy * dy );

					if ( d < 2.f ) {
						if ( d < 0.03f ) {
							vertices[i+1].x = eeRandf( -1, 1 ) * aspectRatio;
							vertices[i+1].y = eeRandf( -1, 1 );
							velocities[i].x = 0;
							velocities[i].y = 0;
						} else {
							dx /= d;
							dy /= d;
							d = ( 2 - d ) * 0.5;
							d *= d;
							velocities[i].x += dx * d * .01;
							velocities[i].y += dy * d * .01;
						}
					}
				}
			}

			/// VertexPointer assigns values by default to the attribute "dgl_Vertex"
			/// TextureCoordPointer to "dgl_MultiTexCoord0"
			GLi->VertexPointer( 3, GL_FP, sizeof(eeVector3ff), reinterpret_cast<char*> ( &vertices[0] ), 0 );

			/// ColorPointer to "dgl_FrontColor"
			GLi->ColorPointer( 4, GL_FP, sizeof(eeColorAf), reinterpret_cast<char*> ( &colors[0] ), 0  );

			/// Draw the lines
			GLi->DrawArrays( DM_LINES, 0, ParticlesNum );

			win->Display();
		}

		eeSAFE_DELETE_ARRAY( vertices );
		eeSAFE_DELETE_ARRAY( velocities );
		eeSAFE_DELETE_ARRAY( colors );
	}

	cEngine::DestroySingleton();

	EE::MemoryManager::LogResults();

	return 0;
}