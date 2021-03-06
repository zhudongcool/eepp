#include <algorithm>
#include <eepp/system/filesystem.hpp>
#include <eepp/ui/models/filesystemmodel.hpp>
#include <eepp/ui/uifiledialog.hpp>
#include <eepp/ui/uilinearlayout.hpp>
#include <eepp/ui/uilistboxitem.hpp>
#include <eepp/ui/uiscenenode.hpp>
#include <eepp/ui/uistyle.hpp>
#include <eepp/ui/uithememanager.hpp>

namespace EE { namespace UI {

#define FDLG_MIN_WIDTH 420
#define FDLG_MIN_HEIGHT 320

UIFileDialog* UIFileDialog::New( Uint32 dialogFlags, const std::string& defaultFilePattern,
								 const std::string& defaultDirectory ) {
	return eeNew( UIFileDialog, ( dialogFlags, defaultFilePattern, defaultDirectory ) );
}

UIFileDialog::UIFileDialog( Uint32 dialogFlags, const std::string& defaultFilePattern,
							const std::string& defaultDirectory ) :
	UIWindow(),
	mCurPath( FileSystem::getRealPath( defaultDirectory ) ),
	mDialogFlags( dialogFlags ),
	mCloseShortcut( KEY_ESCAPE ) {
	if ( getSize().getWidth() < FDLG_MIN_WIDTH ) {
		mDpSize.x = FDLG_MIN_WIDTH;
		mSize.x = PixelDensity::dpToPxI( FDLG_MIN_WIDTH );
	}

	if ( getSize().getHeight() < FDLG_MIN_HEIGHT ) {
		mDpSize.y = FDLG_MIN_HEIGHT;
		mSize.y = PixelDensity::dpToPxI( FDLG_MIN_HEIGHT );
	}

	if ( mStyleConfig.MinWindowSize.getWidth() < FDLG_MIN_WIDTH )
		mStyleConfig.MinWindowSize.setWidth( FDLG_MIN_WIDTH );

	if ( mStyleConfig.MinWindowSize.getHeight() < FDLG_MIN_HEIGHT )
		mStyleConfig.MinWindowSize.setHeight( FDLG_MIN_HEIGHT );

	bool loading = isSceneNodeLoading();
	mUISceneNode->setIsLoading( true );

	mContainer->setSize( getSize() );

	if ( getAllowFolderSelect() ) {
		setTitle( "Select a folder" );
	} else {
		setTitle( "Select a file" );
	}

	UILinearLayout* linearLayout = UILinearLayout::NewVertical();
	linearLayout->setLayoutSizePolicy( SizePolicy::MatchParent, SizePolicy::MatchParent )
		->setLayoutMargin( Rectf( 4, 2, 4, 2 ) )
		->setParent( getContainer() );

	UILinearLayout* hLayout = UILinearLayout::NewHorizontal();
	hLayout->setLayoutSizePolicy( SizePolicy::MatchParent, SizePolicy::WrapContent )
		->setLayoutMargin( Rectf( 0, 0, 0, 4 ) )
		->setParent( linearLayout )
		->setId( "lay1" );

	UITextView::New()
		->setText( "Look in:" )
		->setLayoutSizePolicy( SizePolicy::WrapContent, SizePolicy::MatchParent )
		->setLayoutMargin( Rectf( 0, 0, 4, 0 ) )
		->setParent( hLayout )
		->setEnabled( false );

	FileSystem::dirAddSlashAtEnd( mCurPath );

	mPath = UITextInput::New();
	mPath->setText( mCurPath )
		->setLayoutSizePolicy( SizePolicy::WrapContent, SizePolicy::WrapContent )
		->setLayoutWeight( 1 )
		->setParent( hLayout );
	mPath->addEventListener( Event::OnPressEnter, cb::Make1( this, &UIFileDialog::onPressEnter ) );

	mButtonUp = UIPushButton::New();
	mButtonUp->setText( "Up" )
		->setLayoutMarginLeft( 4 )
		->setLayoutSizePolicy( SizePolicy::WrapContent, SizePolicy::MatchParent )
		->setParent( hLayout );

	mList = UIListView::New();
	mList->setParent( linearLayout );
	mList->setLayoutSizePolicy( SizePolicy::MatchParent, SizePolicy::WrapContent )
		->setLayoutWeight( 1 )
		->setLayoutMargin( Rectf( 0, 0, 0, 4 ) );
	mList->addEventListener( Event::KeyDown, [&]( const Event* event ) {
		const KeyEvent* KEvent = reinterpret_cast<const KeyEvent*>( event );
		if ( KEvent->getKeyCode() == KEY_BACKSPACE ) {
			goFolderUp();
		}
	} );
	mList->addEventListener( Event::OnModelEvent, [&]( const Event* event ) {
		const ModelEvent* modelEvent = static_cast<const ModelEvent*>( event );
		if ( modelEvent->getModelEventType() == ModelEventType::Open ) {
			Variant vPath(
				modelEvent->getModel()->data( modelEvent->getModelIndex(), Model::Role::Custom ) );
			if ( vPath.isValid() && vPath.is( Variant::Type::cstr ) ) {
				bool shouldOpenFolder = false;
				if ( getAllowFolderSelect() && modelEvent->getTriggerEvent() &&
					 modelEvent->getTriggerEvent()->getType() == Event::EventType::KeyDown ) {
					const KeyEvent* keyEvent =
						static_cast<const KeyEvent*>( modelEvent->getTriggerEvent() );
					if ( keyEvent->getMod() & KEYMOD_CTRL ) {
						shouldOpenFolder = true;
					}
				}
				openFileOrFolder( shouldOpenFolder );
			}
		}
	} );
	mList->setOnSelectionChange( [&] {
		if ( mList->getSelection().isEmpty() )
			return;
		auto* node = (FileSystemModel::Node*)mList->getSelection().first().data();
		if ( !isSaveDialog() ) {
			if ( getAllowFolderSelect() || !FileSystem::isDirectory( node->fullPath() ) )
				setFileName( node->getName() );
		} else if ( !FileSystem::isDirectory( node->fullPath() ) ) {
			setFileName( node->getName() );
		}
	} );
	mList->setAutoExpandOnSingleColumn( true );

	hLayout = UILinearLayout::NewHorizontal();
	hLayout->setLayoutSizePolicy( SizePolicy::MatchParent, SizePolicy::WrapContent )
		->setLayoutMargin( Rectf( 0, 0, 0, 4 ) )
		->setParent( linearLayout );

	UITextView::New()
		->setText( "File Name:" )
		->setLayoutSizePolicy( SizePolicy::Fixed, SizePolicy::MatchParent )
		->setSize( 74, 0 )
		->setParent( hLayout )
		->setEnabled( false );

	mFile = UITextInput::New();
	mFile->setLayoutSizePolicy( SizePolicy::WrapContent, SizePolicy::MatchParent )
		->setLayoutWeight( 1 )
		->setParent( hLayout );
	mFile->setLayoutMargin( Rectf( 0, 0, 4, 0 ) );
	mFile->addEventListener( Event::OnPressEnter,
							 cb::Make1( this, &UIFileDialog::onPressFileEnter ) );

	mButtonOpen = UIPushButton::New();
	mButtonOpen->setText( isSaveDialog() ? "Save" : "Open" )
		->setLayoutSizePolicy( SizePolicy::Fixed, SizePolicy::WrapContent )
		->setSize( 80, 0 )
		->setParent( hLayout );

	hLayout = UILinearLayout::NewHorizontal();
	hLayout->setLayoutSizePolicy( SizePolicy::MatchParent, SizePolicy::WrapContent )
		->setParent( linearLayout );

	UITextView::New()
		->setText( "Files of type:" )
		->setLayoutSizePolicy( SizePolicy::Fixed, SizePolicy::MatchParent )
		->setSize( 74, 0 )
		->setParent( hLayout )
		->setEnabled( false );

	mFiletype = UIDropDownList::New();
	mFiletype->setLayoutSizePolicy( SizePolicy::WrapContent, SizePolicy::WrapContent )
		->setLayoutWeight( 1 )
		->setParent( hLayout );
	mFiletype->setPopUpToRoot( true );
	mFiletype->getListBox()->addListBoxItem( defaultFilePattern );
	mFiletype->getListBox()->setSelected( 0 );
	mFiletype->setLayoutMargin( Rectf( 0, 0, 4, 0 ) );

	mButtonCancel = UIPushButton::New();
	mButtonCancel->setText( "Cancel" )
		->setLayoutSizePolicy( SizePolicy::Fixed, SizePolicy::WrapContent )
		->setSize( 80, 0 )
		->setParent( hLayout );

	mList->setFocus();

	applyDefaultTheme();

	mUISceneNode->setIsLoading( loading );
}

UIFileDialog::~UIFileDialog() {}

void UIFileDialog::onWindowReady() {
	updateClickStep();
}

Uint32 UIFileDialog::getType() const {
	return UI_TYPE_FILEDIALOG;
}

bool UIFileDialog::isType( const Uint32& type ) const {
	return UIFileDialog::getType() == type ? true : UIWindow::isType( type );
}

void UIFileDialog::setTheme( UITheme* Theme ) {
	UIWindow::setTheme( Theme );

	mButtonOpen->setTheme( Theme );
	mButtonCancel->setTheme( Theme );
	mButtonUp->setTheme( Theme );
	mList->setTheme( Theme );
	mPath->setTheme( Theme );
	mFile->setTheme( Theme );
	mFiletype->setTheme( Theme );

	Drawable* icon = getUISceneNode()->findIconDrawable( "go-up", PixelDensity::dpToPxI( 16 ) );

	if ( NULL != icon ) {
		mButtonUp->setText( "" );
		mButtonUp->setIcon( icon );
	}

	onThemeLoaded();
}

void UIFileDialog::refreshFolder() {
	FileSystem::dirAddSlashAtEnd( mCurPath );
	std::vector<String> flist = FileSystem::filesGetInPath(
		String( mCurPath ), getSortAlphabetically(), getFoldersFirst(), !getShowHidden() );
	std::vector<String> files;
	std::vector<std::string> patterns;

	if ( "*" != mFiletype->getText() ) {
		patterns = String::split( mFiletype->getText().toUtf8(), ';' );

		for ( size_t i = 0; i < patterns.size(); i++ )
			patterns[i] = FileSystem::fileExtension( String::trim( patterns[i] ) );
	}

	mList->setModel( FileSystemModel::New(
		mCurPath,
		getShowOnlyFolders() ? FileSystemModel::Mode::DirectoriesOnly
							 : FileSystemModel::Mode::FilesAndDirectories,
		FileSystemModel::DisplayConfig( getSortAlphabetically(), getFoldersFirst(),
										!getShowHidden(), patterns ) ) );

	mList->setColumnsVisible( { FileSystemModel::Name } );
	mList->setHeadersVisible( false );

	updateClickStep();

	mList->setFocus();
}

void UIFileDialog::updateClickStep() {
	if ( NULL != mList->getVerticalScrollBar() ) {
		mList->getVerticalScrollBar()->setClickStep(
			1.f / ( ( mList->getModel()->rowCount() * mList->getRowHeight() ) /
					(Float)mList->getSize().getHeight() ) );
	}
}

void UIFileDialog::setCurPath( const std::string& path ) {
	mCurPath = path;
	FileSystem::dirAddSlashAtEnd( mCurPath );
	mPath->setText( mCurPath );
	if ( !isSaveDialog() )
		mFile->setText( "" );
	refreshFolder();
}

void UIFileDialog::openSaveClick() {
	if ( isSaveDialog() ) {
		save();
	} else {
		open();
	}
}

void UIFileDialog::onPressFileEnter( const Event* ) {
	openSaveClick();
}

void UIFileDialog::disableButtons() {
	mButtonOpen->setEnabled( false );
	mButtonCancel->setEnabled( false );
	mButtonUp->setEnabled( false );

	if ( NULL != mButtonClose )
		mButtonClose->setEnabled( false );

	if ( NULL != mButtonMinimize )
		mButtonMinimize->setEnabled( false );

	if ( NULL != mButtonMaximize )
		mButtonMaximize->setEnabled( false );
}

void UIFileDialog::openFileOrFolder( bool shouldOpenFolder = false ) {
	if ( mList->getSelection().isEmpty() )
		return;
	auto* node = (FileSystemModel::Node*)mList->getSelection().first().data();

	std::string newPath = mCurPath + node->getName();

	if ( FileSystem::isDirectory( newPath ) ) {
		if ( shouldOpenFolder ) {
			open();
		} else {
			setCurPath( newPath );
		}
	} else {
		open();
	}
}

void UIFileDialog::goFolderUp() {
	std::string prevFolderName( FileSystem::fileNameFromPath( mCurPath ) );
	setCurPath( FileSystem::removeLastFolderFromPath( mCurPath ) );
	ModelIndex index = mList->findRowWithText( prevFolderName );
	if ( index.isValid() )
		mList->setSelection( index );
}

Uint32 UIFileDialog::onMessage( const NodeMessage* Msg ) {
	switch ( Msg->getMsg() ) {
		case NodeMessage::MouseClick: {
			if ( Msg->getFlags() & EE_BUTTON_LMASK ) {
				if ( Msg->getSender() == mButtonOpen ) {
					openSaveClick();
				} else if ( Msg->getSender() == mButtonCancel ) {
					disableButtons();

					closeWindow();
				} else if ( Msg->getSender() == mButtonUp ) {
					goFolderUp();
				}
			}

			break;
		}
		case NodeMessage::MouseDoubleClick: {
			if ( Msg->getFlags() & EE_BUTTON_LMASK ) {
				if ( Msg->getSender()->isType( UI_TYPE_LISTBOXITEM ) ) {
					openFileOrFolder();
				}
			}

			break;
		}
		case NodeMessage::Selected: {
			if ( Msg->getSender() == mFiletype ) {
				refreshFolder();
			}

			break;
		}
	}

	return UIWindow::onMessage( Msg );
}

void UIFileDialog::save() {
	sendCommonEvent( Event::SaveFile );

	disableButtons();

	closeWindow();
}

void UIFileDialog::open() {
	if ( mList->getSelection().isEmpty() &&
		 !( getAllowFolderSelect() && FileSystem::isDirectory( getFullPath() ) ) )
		return;

	auto* node = !mList->getSelection().isEmpty()
					 ? (FileSystemModel::Node*)mList->getSelection().first().data()
					 : nullptr;

	if ( ( node && "" != node->getName() ) || getAllowFolderSelect() ) {
		if ( !getAllowFolderSelect() ) {
			if ( FileSystem::isDirectory( getFullPath() ) )
				return;
		} else {
			if ( !FileSystem::isDirectory( getFullPath() ) &&
				 !FileSystem::isDirectory( getCurPath() ) )
				return;
		}

		sendCommonEvent( Event::OpenFile );

		disableButtons();

		closeWindow();
	}
}

void UIFileDialog::onPressEnter( const Event* ) {
	if ( FileSystem::isDirectory( mPath->getText() ) ) {
		setCurPath( mPath->getText() );
	}
}

void UIFileDialog::addFilePattern( std::string pattern, bool select ) {
	Uint32 index = mFiletype->getListBox()->addListBoxItem( pattern );

	if ( select ) {
		mFiletype->getListBox()->setSelected( index );

		refreshFolder();
	}
}

bool UIFileDialog::isSaveDialog() {
	return 0 != ( mDialogFlags & SaveDialog );
}

bool UIFileDialog::getSortAlphabetically() {
	return 0 != ( mDialogFlags & SortAlphabetically );
}

bool UIFileDialog::getFoldersFirst() {
	return 0 != ( mDialogFlags & FoldersFirst );
}

bool UIFileDialog::getAllowFolderSelect() {
	return 0 != ( mDialogFlags & AllowFolderSelect );
}

bool UIFileDialog::getShowOnlyFolders() {
	return 0 != ( mDialogFlags & ShowOnlyFolders );
}

bool UIFileDialog::getShowHidden() {
	return 0 != ( mDialogFlags & ShowHidden );
}

void UIFileDialog::setSortAlphabetically( const bool& sortAlphabetically ) {
	BitOp::setBitFlagValue( &mDialogFlags, SortAlphabetically, sortAlphabetically ? 1 : 0 );
	refreshFolder();
}

void UIFileDialog::setFoldersFirst( const bool& foldersFirst ) {
	BitOp::setBitFlagValue( &mDialogFlags, FoldersFirst, foldersFirst ? 1 : 0 );
	refreshFolder();
}

void UIFileDialog::setAllowFolderSelect( const bool& allowFolderSelect ) {
	BitOp::setBitFlagValue( &mDialogFlags, AllowFolderSelect, allowFolderSelect ? 1 : 0 );
}

void UIFileDialog::setShowOnlyFolders( const bool& showOnlyFolders ) {
	BitOp::setBitFlagValue( &mDialogFlags, ShowOnlyFolders, showOnlyFolders ? 1 : 0 );
	refreshFolder();
}

void UIFileDialog::setShowHidden( const bool& showHidden ) {
	BitOp::setBitFlagValue( &mDialogFlags, ShowHidden, showHidden ? 1 : 0 );
	refreshFolder();
}

std::string UIFileDialog::getFullPath() {
	std::string tPath = mCurPath;

	FileSystem::dirAddSlashAtEnd( tPath );

	tPath += getCurFile();

	return tPath;
}

std::string UIFileDialog::getCurPath() const {
	return mCurPath;
}

std::string UIFileDialog::getCurFile() const {
	if ( mDialogFlags & SaveDialog )
		return mFile->getText();
	if ( mList->getSelection().isEmpty() )
		return "";
	auto* node = (FileSystemModel::Node*)mList->getSelection().first().data();
	return node->getName();
}

UIPushButton* UIFileDialog::getButtonOpen() const {
	return mButtonOpen;
}

UIPushButton* UIFileDialog::getButtonCancel() const {
	return mButtonCancel;
}

UIPushButton* UIFileDialog::getButtonUp() const {
	return mButtonUp;
}

UIListView* UIFileDialog::getList() const {
	return mList;
}

UITextInput* UIFileDialog::getPathInput() const {
	return mPath;
}

UITextInput* UIFileDialog::getFileInput() const {
	return mFile;
}

UIDropDownList* UIFileDialog::getFiletypeList() const {
	return mFiletype;
}

Uint32 UIFileDialog::onKeyUp( const KeyEvent& event ) {
	if ( mCloseShortcut && event.getKeyCode() == mCloseShortcut &&
		 ( mCloseShortcut.mod == 0 || ( event.getMod() & mCloseShortcut.mod ) ) ) {
		disableButtons();

		closeWindow();
	}

	return UIWindow::onKeyUp( event );
}

const KeyBindings::Shortcut& UIFileDialog::getCloseShortcut() const {
	return mCloseShortcut;
}

void UIFileDialog::setFileName( const std::string& name ) {
	if ( mFile )
		mFile->setText( name );
}

void UIFileDialog::setCloseShortcut( const KeyBindings::Shortcut& closeWithKey ) {
	mCloseShortcut = closeWithKey;
}

}} // namespace EE::UI
