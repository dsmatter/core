/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <svtools/popupwindowcontroller.hxx>
#include <svtools/toolbarmenu.hxx>
#include <svtools/valueset.hxx>
#include <vcl/toolbox.hxx>

#include "strings.hrc"
#include "res_bmp.hrc"
#include "sdresid.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;

namespace sd
{

// Component to select which display mode has to be used.
// Composed of a dropdown button in the toolbar and a
// popup menu to select the value

class DisplayModeController : public svt::PopupWindowController
{
public:
    explicit DisplayModeController( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    virtual VclPtr<vcl::Window> createPopupWindow( vcl::Window* pParent ) override;

    // XInitialization
    virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
        throw ( css::uno::Exception, css::uno::RuntimeException, std::exception ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw ( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw ( css::uno::RuntimeException, std::exception ) override;

    void setToolboxItemImage( sal_uInt16 nImage );

    using svt::PopupWindowController::createPopupWindow;
};

class DisplayModeToolbarMenu : public svtools::ToolbarMenu
{
public:
    DisplayModeToolbarMenu( DisplayModeController& rController,
        const Reference< XFrame >& xFrame, vcl::Window* pParent );
    virtual ~DisplayModeToolbarMenu() override;
    virtual void dispose() override;

protected:
    DECL_LINK( SelectToolbarMenuHdl, ToolbarMenu*, void );
    DECL_LINK( SelectValueSetHdl, ValueSet*, void );
    void SelectHdl(void*);

private:
    DisplayModeController& mrController;
    VclPtr<ValueSet> mpDisplayModeSet1;
    VclPtr<ValueSet> mpDisplayModeSet2;
};

struct snewfoil_value_info
{
    sal_uInt16  mnId;
    sal_uInt16  mnBmpResId;
    sal_uInt16  mnStrResId;
    OUString    msUnoCommand;
};

static const snewfoil_value_info editmodes[] =
{
    {1,
        BMP_DISPLAYMODE_SLIDE,
        STR_NORMAL_MODE,
        OUString(".uno:NormalMultiPaneGUI") },
    {2,
        BMP_DISPLAYMODE_NOTES,
        STR_NOTES_MODE,
        OUString(".uno:NotesMode") },
    {3,
        BMP_DISPLAYMODE_OUTLINE,
        STR_OUTLINE_MODE,
        OUString(".uno:OutlineMode") },
    {4,
        BMP_DISPLAYMODE_SLIDE_SORTER,
        STR_SLIDE_SORTER_MODE,
        OUString(".uno:DiaMode") },
    {0, 0, 0, OUString() }
};

static const snewfoil_value_info mastermodes[] =
{
    {5,
        BMP_DISPLAYMODE_SLIDE_MASTER,
        STR_SLIDE_MASTER_MODE,
        OUString(".uno:SlideMasterPage") },
    {6,
        BMP_DISPLAYMODE_NOTES_MASTER,
        STR_NOTES_MASTER_MODE,
        OUString(".uno:NotesMasterPage") },
    {7,
        BMP_DISPLAYMODE_HANDOUT_MASTER,
        STR_HANDOUT_MASTER_MODE,
        OUString(".uno:HandoutMode") },
    {0, 0, 0, OUString() }
};


static void fillLayoutValueSet( ValueSet* pValue, const snewfoil_value_info* pInfo )
{
    Size aLayoutItemSize;
    for( ; pInfo->mnId; pInfo++ )
    {
        OUString aText( SD_RESSTR( pInfo->mnStrResId ) );
        BitmapEx aBmp(  SdResId( pInfo->mnBmpResId ) );

        pValue->InsertItem(pInfo->mnId, Image(aBmp), aText);

        aLayoutItemSize.Width()  = std::max( aLayoutItemSize.Width(),  aBmp.GetSizePixel().Width()  );
        aLayoutItemSize.Height() = std::max( aLayoutItemSize.Height(), aBmp.GetSizePixel().Height() );
    }

    aLayoutItemSize = pValue->CalcItemSizePixel( aLayoutItemSize );
    pValue->SetSizePixel( pValue->CalcWindowSizePixel( aLayoutItemSize ) );
}

DisplayModeToolbarMenu::DisplayModeToolbarMenu( DisplayModeController& rController,
    const Reference< XFrame >& xFrame, vcl::Window* pParent )
: svtools::ToolbarMenu(xFrame, pParent, WB_CLIPCHILDREN )
, mrController( rController )
, mpDisplayModeSet1( nullptr )
, mpDisplayModeSet2( nullptr )
{
    const sal_Int32 LAYOUT_BORDER_PIX = 7;

    OUString aTitle1( SD_RESSTR( STR_DISPLAYMODE_EDITMODES ) );
    OUString aTitle2( SD_RESSTR( STR_DISPLAYMODE_MASTERMODES ) );

    SetSelectHdl( LINK( this, DisplayModeToolbarMenu, SelectToolbarMenuHdl ) );

    mpDisplayModeSet1 = createEmptyValueSetControl();
    mpDisplayModeSet1->SetSelectHdl( LINK( this, DisplayModeToolbarMenu, SelectValueSetHdl ) );

    sal_Int16 nColCount = 2;
    mpDisplayModeSet1->SetColCount( nColCount );
    fillLayoutValueSet( mpDisplayModeSet1, &editmodes[0] );

    Size aSize( mpDisplayModeSet1->GetOutputSizePixel() );
    aSize.Width() += (mpDisplayModeSet1->GetColCount() + 1) * LAYOUT_BORDER_PIX;
    aSize.Height() += (mpDisplayModeSet1->GetLineCount() +1) * LAYOUT_BORDER_PIX;
    mpDisplayModeSet1->SetOutputSizePixel( aSize );

    appendEntry( -1, aTitle1 );
    appendEntry( 1, mpDisplayModeSet1 );

    mpDisplayModeSet2 = createEmptyValueSetControl();

    mpDisplayModeSet2->SetSelectHdl( LINK( this, DisplayModeToolbarMenu, SelectValueSetHdl ) );
    mpDisplayModeSet2->SetColCount( nColCount );

    fillLayoutValueSet( mpDisplayModeSet2, &mastermodes[0] );

    aSize = mpDisplayModeSet2->GetOutputSizePixel();
    aSize.Width() += (mpDisplayModeSet2->GetColCount() + 1) * LAYOUT_BORDER_PIX;
    aSize.Height() += (mpDisplayModeSet2->GetLineCount() + 1) * LAYOUT_BORDER_PIX;
    mpDisplayModeSet2->SetOutputSizePixel( aSize );

    appendEntry( -1, aTitle2 );
    appendEntry( 2, mpDisplayModeSet2 );

    SetOutputSizePixel( getMenuSize() );
}

DisplayModeToolbarMenu::~DisplayModeToolbarMenu()
{
    disposeOnce();
}

void DisplayModeToolbarMenu::dispose()
{
    mpDisplayModeSet1.clear();
    mpDisplayModeSet2.clear();
    svtools::ToolbarMenu::dispose();
}

IMPL_LINK( DisplayModeToolbarMenu, SelectValueSetHdl, ValueSet*, pControl, void )
{
    SelectHdl(pControl);
}
IMPL_LINK( DisplayModeToolbarMenu, SelectToolbarMenuHdl, ToolbarMenu *, pControl, void )
{
    SelectHdl(pControl);
}

void DisplayModeToolbarMenu::SelectHdl(void * pControl)
{
    if ( IsInPopupMode() )
        EndPopupMode();

    OUString sCommandURL;
    sal_uInt16 nImage = 0;

    if( pControl == mpDisplayModeSet1 ) {
        sCommandURL = editmodes[mpDisplayModeSet1->GetSelectItemId() - 1 ].msUnoCommand;
        nImage = editmodes[mpDisplayModeSet1->GetSelectItemId() - 1 ].mnBmpResId;
    }
    else if( pControl == mpDisplayModeSet2 ) {
        sCommandURL = mastermodes[mpDisplayModeSet2->GetSelectItemId() - 5 ].msUnoCommand;
        nImage = mastermodes[mpDisplayModeSet2->GetSelectItemId() - 5 ].mnBmpResId;
    }

    if (!sCommandURL.isEmpty())
        mrController.dispatchCommand( sCommandURL, Sequence< PropertyValue >() );

    mrController.setToolboxItemImage( nImage );
}

DisplayModeController::DisplayModeController( const css::uno::Reference< css::uno::XComponentContext >& rxContext )
: svt::PopupWindowController( rxContext, Reference< frame::XFrame >(), OUString() )
{
}

void SAL_CALL DisplayModeController::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
        throw ( css::uno::Exception, css::uno::RuntimeException, std::exception )
{
    svt::PopupWindowController::initialize( aArguments );
    ToolBox* pToolBox = nullptr;
    sal_uInt16 nId = 0;
    if ( getToolboxId( nId, &pToolBox ) )
        pToolBox->SetItemBits( nId, pToolBox->GetItemBits( nId ) | ToolBoxItemBits::DROPDOWNONLY );
    setToolboxItemImage( BMP_DISPLAYMODE_SLIDE );
}

VclPtr<vcl::Window> DisplayModeController::createPopupWindow( vcl::Window* pParent )
{
    return VclPtr<sd::DisplayModeToolbarMenu>::Create( *this, m_xFrame, pParent );
}

void DisplayModeController::setToolboxItemImage( sal_uInt16 nImage )
{
    sal_uInt16 nId;
    ToolBox* pToolBox = nullptr;
    if (getToolboxId( nId, &pToolBox )) {
        SdResId resId( nImage );
        BitmapEx aBmp( resId );
        int targetSize = (pToolBox->GetToolboxButtonSize() == ToolBoxButtonSize::Large) ? 32 : 16;
        double scale = 1.0f;
        Size size = aBmp.GetSizePixel();
        if (size.Width() > targetSize)
            scale = (double)targetSize / (double)size.Width();
        if (size.Height() > targetSize)
            scale = ::std::min( scale, (double)targetSize / (double)size.Height() );
        aBmp.Scale( scale, scale );
        pToolBox->SetItemImage( nId, Image( aBmp ) );
    }
}

// XServiceInfo

OUString SAL_CALL DisplayModeController::getImplementationName() throw( RuntimeException, std::exception )
{
    return OUString( "com.sun.star.comp.sd.DisplayModeController" );
}

Sequence< OUString > SAL_CALL DisplayModeController::getSupportedServiceNames(  ) throw( RuntimeException, std::exception )
{
    css::uno::Sequence<OUString> aRet { "com.sun.star.frame.ToolbarController" };
    return aRet;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT ::com::sun::star::uno::XInterface* SAL_CALL
com_sun_star_comp_sd_DisplayModeController_get_implementation( css::uno::XComponentContext* context,
                                                               css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new sd::DisplayModeController(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
