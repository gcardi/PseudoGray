object frmMain: TfrmMain
  Left = 0
  Top = 0
  Caption = 'Pseudo Gray Image Viewer'
  ClientHeight = 817
  ClientWidth = 1009
  Color = clBtnFace
  DoubleBuffered = True
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = 'Segoe UI'
  Font.Style = []
  TextHeight = 15
  object PageControl1: TPageControl
    Left = 0
    Top = 54
    Width = 1009
    Height = 763
    ActivePage = tbshtOriginal
    Align = alClient
    TabOrder = 0
    object tbshtOriginal: TTabSheet
      Caption = 'Original'
      DoubleBuffered = False
      ImageIndex = 2
      ParentDoubleBuffered = False
      OnResize = tbshtOriginalResize
      object ScrollBox1: TScrollBox
        Left = 0
        Top = 0
        Width = 1001
        Height = 733
        HorzScrollBar.Tracking = True
        VertScrollBar.Tracking = True
        Align = alClient
        TabOrder = 0
        object paintboxOriginal: TPaintBox
          Left = 0
          Top = 0
          Width = 1920
          Height = 1280
          OnPaint = paintboxOriginalPaint
        end
      end
    end
    object tbshtPseudoGrey: TTabSheet
      Caption = 'Pseudo Gray'
      object ScrollBox2: TScrollBox
        Left = 0
        Top = 0
        Width = 1001
        Height = 733
        HorzScrollBar.Tracking = True
        VertScrollBar.Tracking = True
        Align = alClient
        TabOrder = 0
        object imgPseudoGrey: TImage
          Left = 0
          Top = 0
          Width = 1920
          Height = 1280
          Cursor = crCross
          OnMouseLeave = ImgMouseLeave
          OnMouseMove = ImgMouseMove
        end
      end
    end
    object tbshtGrey: TTabSheet
      Caption = 'Gray'
      ImageIndex = 1
      object ScrollBox3: TScrollBox
        Left = 0
        Top = 0
        Width = 1001
        Height = 733
        HorzScrollBar.Tracking = True
        VertScrollBar.Tracking = True
        Align = alClient
        TabOrder = 0
        object imgGrey: TImage
          Left = 0
          Top = 0
          Width = 1920
          Height = 1280
          Cursor = crCross
          OnMouseLeave = ImgMouseLeave
          OnMouseMove = ImgMouseMove
        end
      end
    end
  end
  object ActionMainMenuBar1: TActionMainMenuBar
    Left = 0
    Top = 0
    Width = 1009
    Height = 25
    UseSystemFont = False
    ActionManager = ActionManager1
    Caption = 'ActionMainMenuBar1'
    Color = clMenuBar
    ColorMap.DisabledFontColor = 10461087
    ColorMap.HighlightColor = clWhite
    ColorMap.BtnSelectedFont = clBlack
    ColorMap.UnusedColor = clWhite
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clBlack
    Font.Height = -12
    Font.Name = 'Segoe UI'
    Font.Style = []
    Spacing = 0
  end
  object Panel1: TPanel
    Left = 0
    Top = 25
    Width = 1009
    Height = 29
    Align = alTop
    BevelOuter = bvNone
    Caption = 'Panel1'
    ShowCaption = False
    TabOrder = 2
    object lblGreyImageInfo: TLabel
      Left = 16
      Top = 6
      Width = 5
      Height = 15
      Caption = '-'
    end
  end
  object ActionManager1: TActionManager
    ActionBars = <
      item
        Items = <
          item
            Items = <
              item
                Action = actFileOpen
                ShortCut = 16463
              end
              item
                Caption = '-'
              end
              item
                Action = actFileExit
                ImageIndex = 43
              end>
            Caption = '&File'
          end>
        ActionBar = ActionMainMenuBar1
      end>
    Left = 712
    Top = 56
    StyleName = 'Platform Default'
    object actFileOpen: TAction
      Category = 'File'
      Caption = '&Open...'
      Hint = 'Open|Opens an existing file'
      ShortCut = 16463
      OnExecute = actFileOpenExecute
    end
    object actFileExit: TFileExit
      Category = 'File'
      Caption = 'E&xit'
      Hint = 'Exit|Quits the application'
      ImageIndex = 43
    end
  end
  object FileOpenDialog1: TFileOpenDialog
    FavoriteLinks = <>
    FileNameLabel = 'Picture file'
    FileTypes = <
      item
        DisplayName = 'TIFF files'
        FileMask = '*.tif;*.tiff'
      end
      item
        DisplayName = 'All files'
        FileMask = '*.*'
      end>
    OkButtonLabel = 'Open'
    Options = [fdoFileMustExist]
    Title = 'Please, select a 16-bit monochrome image'
    Left = 600
    Top = 49
  end
end
