#include <eepp/core.hpp>
#include <eepp/ui/abstract/uiabstractview.hpp>
#include <eepp/ui/models/modelselection.hpp>

namespace EE { namespace UI { namespace Models {

void ModelSelection::removeMatching( std::function<bool( const ModelIndex& )> filter ) {
	std::vector<std::vector<ModelIndex>::iterator> toRemove;
	for ( auto it = mIndexes.begin(); it != mIndexes.end(); it++ ) {
		if ( filter( *it ) )
			toRemove.push_back( it );
	}
	for ( auto& index : toRemove )
		mIndexes.erase( index );
}

void ModelSelection::set( const ModelIndex& index ) {
	eeASSERT( index.isValid() );
	if ( mIndexes.size() == 1 && contains( index ) )
		return;
	mIndexes.clear();
	mIndexes.push_back( index );
	mView->notifySelectionChange();
}

void ModelSelection::add( const ModelIndex& index ) {
	eeASSERT( index.isValid() );
	auto contains = std::find( mIndexes.begin(), mIndexes.end(), index );
	if ( contains == mIndexes.end() )
		return;
	mIndexes.push_back( index );
	mView->notifySelectionChange();
}

void ModelSelection::toggle( const ModelIndex& index ) {
	eeASSERT( index.isValid() );
	auto contains = std::find( mIndexes.begin(), mIndexes.end(), index );
	if ( contains != mIndexes.end() )
		mIndexes.erase( contains );
	else
		mIndexes.push_back( index );
	mView->notifySelectionChange();
}

bool ModelSelection::remove( const ModelIndex& index ) {
	eeASSERT( index.isValid() );
	auto contains = std::find( mIndexes.begin(), mIndexes.end(), index );
	if ( contains == mIndexes.end() )
		return false;
	mIndexes.erase( contains );
	mView->notifySelectionChange();
	return true;
}

void ModelSelection::clear() {
	if ( mIndexes.empty() )
		return;
	mIndexes.clear();
	mView->notifySelectionChange();
}

void ModelSelection::notifySelectionChanged() {
	if ( !mDisableNotify ) {
		mView->notifySelectionChange();
		mNotifyPending = false;
	} else {
		mNotifyPending = true;
	}
}

}}} // namespace EE::UI::Models
