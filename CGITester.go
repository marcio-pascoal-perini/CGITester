/*

Windows

go mod init CGITester

go mod tidy

go build -ldflags "-s -w -H windowsgui" -o CGITester.exe CGITester.go

.\CGITester.exe

Linux

go mod init CGITester

go mod tidy

go build -o CGITester.exe CGITester.go

./CGITester.exe

./CGITester.exe > /dev/null 2>&1 &

*/

package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"io"
	"log"
	"net/http"
	"net/http/cgi"
	"os"
	"path/filepath"
	"strconv"
	"strings"

	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/app"
	"fyne.io/fyne/v2/container"
	"fyne.io/fyne/v2/dialog"
	"fyne.io/fyne/v2/layout"
	"fyne.io/fyne/v2/theme"
	"fyne.io/fyne/v2/widget"
	"github.com/zserge/lorca"
)

type Parameter struct {
	ID              string
	Description     string
	ExecutableFile  string
	InitialHTMLFile string
	Argument01      string
	Argument02      string
	Argument03      string
	Argument04      string
	Argument05      string
}

var parameters []Parameter

func (p *Parameter) Create() {
	parameters = append(parameters, *p)
}

func (p *Parameter) Delete() bool {
	for i, item := range parameters {
		if item.ID == p.ID {
			parameters = append(parameters[:i], parameters[i+1:]...)
			return true
		}
	}
	return false
}

func (p *Parameter) Read() bool {
	for _, item := range parameters {
		if item.ID == p.ID {
			p.Description = item.Description
			p.ExecutableFile = item.ExecutableFile
			p.InitialHTMLFile = item.InitialHTMLFile
			p.Argument01 = item.Argument01
			p.Argument02 = item.Argument02
			p.Argument03 = item.Argument03
			p.Argument04 = item.Argument04
			p.Argument05 = item.Argument05
			return true
		}
	}
	return false
}

func (p *Parameter) Update() bool {
	for i, item := range parameters {
		if item.ID == p.ID {
			parameters[i].Description = p.Description
			parameters[i].ExecutableFile = p.ExecutableFile
			parameters[i].InitialHTMLFile = p.InitialHTMLFile
			parameters[i].Argument01 = p.Argument01
			parameters[i].Argument02 = p.Argument02
			parameters[i].Argument03 = p.Argument03
			parameters[i].Argument04 = p.Argument04
			parameters[i].Argument05 = p.Argument05
			return true
		}
	}
	return false
}

const (
	COLUMNS_TOTAL = 9
)

var (
	browser        lorca.UI
	host           *string
	port           *int
	selectedColumn int
	selectedRow    int
	window         fyne.Window
)

func basePath() string {
	dir, err := filepath.Abs(filepath.Dir(os.Args[0]))
	if err != nil {
		log.Fatal(err.Error())
	}
	return dir
}

func cgiHandler(w http.ResponseWriter, r *http.Request) {
	handler := cgi.Handler{
		Path: parameters[selectedRow].ExecutableFile,
		Args: []string{
			parameters[selectedRow].Argument01,
			parameters[selectedRow].Argument02,
			parameters[selectedRow].Argument03,
			parameters[selectedRow].Argument04,
			parameters[selectedRow].Argument05,
		},
	}
	handler.ServeHTTP(w, r)
}

func closeBrowser() {
	if browser != nil {
		browser.Close()
	}
}

func dirDialog(w *widget.Entry) {
	d := dialog.NewFolderOpen(func(list fyne.ListableURI, err error) {
		if err == nil && list != nil {
			w.SetText(list.Path())
		}
	}, window)
	d.Show()
}

func fileDialog(w *widget.Entry) {
	d := dialog.NewFileOpen(func(reader fyne.URIReadCloser, err error) {
		if err == nil && reader != nil {
			w.SetText(reader.URI().Path())
		}
	}, window)
	d.Show()
}

func index(w http.ResponseWriter, r *http.Request) {
	fp := parameters[selectedRow].InitialHTMLFile
	if len(fp) == 0 {
		w.Header().Set("Content-Type", "text/html; charset=UTF-8")
		w.Header().Set("Access-Control-Allow-Origin", "*")
		w.Header().Set("Access-Control-Allow-Headers", "*")
		w.Header().Set("Access-Control-Allow-Methods", "*")
		s := `
		<!DOCTYPE html>
        <html lang="en-US">
        <head>
	    <meta charset="utf-8">
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <title></title>
		<style></style>
		<script>
		window.onload=function(){
		document.getElementById('mainForm').submit();
		};
        </script>
        </head>
		<body>
		<form id="mainForm" name="mainForm" action="/registered-cgi/">
        </form>
		</body>	
		</html>	
		`
		w.Write([]byte(s))
	} else {
		http.ServeFile(w, r, fp)
	}
}

/*
func index(w http.ResponseWriter, r *http.Request) {
	fp := parameters[selectedRow].InitialHTMLFile
	if len(fp) == 0 {
		w.Header().Set("Content-Type", "text/html; charset=UTF-8")
		w.Header().Set("Access-Control-Allow-Origin", "*")
		w.Header().Set("Access-Control-Allow-Headers", "*")
		w.Header().Set("Access-Control-Allow-Methods", "*")
		s := `
		<!DOCTYPE html>
        <html lang="en-US">
        <head>
	    <meta charset="utf-8">
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <title></title>
		<style>
        input[type="submit"] {height: 25px; width: 200px;}
		</style>
        </head>
		<body>
		<br>
		<form id="form1" name="form1" action="/registered-cgi/" method="GET">
        <input id="input1" name="input1" type="submit" value="Start with GET">
        </form>
		<br>
		<form id="form2" name="form2" action="/registered-cgi/" method="POST">
        <input id="input2" name="input2" type="submit" value="Start with POST">
        </form>		
		</body>	
		</html>	
		`
		w.Write([]byte(s))
	} else {
		http.ServeFile(w, r, fp)
	}
}
*/

func loadParameters() {
	filename := filepath.Join(basePath(), "data", "parameters.json")
	_, err := os.Stat(filename)
	if err != nil {
		if os.IsNotExist(err) {
			newFile, _ := os.Create(filename)
			newFile.WriteString(`
			[{
            	"ID": "ID",
            	"Description": "Description",
            	"ExecutableFile": "Executable file",
            	"InitialHTMLFile": "Initial HTML file",
            	"Argument01": "Argument 01",
            	"Argument02": "Argument 02",
            	"Argument03": "Argument 03",
            	"Argument04": "Argument 04",
            	"Argument05": "Argument 05"
            }]
			`)
			newFile.Close()
		}
	}
	jsonFile, err := os.Open(filename)
	if err != nil {
		log.Fatal(err.Error())
	}
	defer jsonFile.Close()
	byteValue, err := io.ReadAll(jsonFile)
	if err != nil {
		log.Fatal(err.Error())
	}
	err = json.Unmarshal(byteValue, &parameters)
	if err != nil {
		log.Fatal(err.Error())
	}
}

func main() {
	go func() {
		host = flag.String("host", "127.0.0.1", "Current host")
		port = flag.Int("port", 3030, "Port number")
		flag.Parse()
		http.HandleFunc("/", index)
		http.Handle("/registered-cgi/", http.StripPrefix("/registered-cgi/", http.HandlerFunc(cgiHandler)))
		http.ListenAndServe(fmt.Sprintf("%s:%d", *host, *port), nil)
	}()
	loadParameters()
	application := app.New()
	window = application.NewWindow("start")
	faviconPath := filepath.Join(basePath(), "assets", "images", "favicon.png")
	resource, err := fyne.LoadResourceFromPath(faviconPath)
	if err == nil {
		window.SetIcon(resource)
	}

	tableInfo := widget.NewTable(
		func() (int, int) {
			return len(parameters), COLUMNS_TOTAL
		},
		func() fyne.CanvasObject {
			label := widget.NewLabel("Template")
			label.Wrapping = fyne.TextWrapWord
			return label
		},
		func(id widget.TableCellID, obj fyne.CanvasObject) {
			label := obj.(*widget.Label)
			parameter := parameters[id.Row]
			switch id.Col {
			case 0:
				label.SetText(parameter.ID)
			case 1:
				label.SetText(parameter.Description)
			case 2:
				label.SetText(parameter.ExecutableFile)
			case 3:
				label.SetText(parameter.InitialHTMLFile)
			case 4:
				label.SetText(parameter.Argument01)
			case 5:
				label.SetText(parameter.Argument02)
			case 6:
				label.SetText(parameter.Argument03)
			case 7:
				label.SetText(parameter.Argument04)
			case 8:
				label.SetText(parameter.Argument05)
			}
			label.Alignment = fyne.TextAlignLeading
			if id.Row == 0 {
				label.TextStyle = fyne.TextStyle{Bold: true}
			} else {
				label.TextStyle = fyne.TextStyle{Bold: false}
			}
			label.Refresh()
		},
	)
	for i := range parameters {
		if i > 0 {
			tableInfo.SetRowHeight(i, 80)
		}
	}
	for i := range COLUMNS_TOTAL {
		if i == 0 {
			tableInfo.SetColumnWidth(i, 50)
		} else {
			tableInfo.SetColumnWidth(i, 300)
		}
	}
	tableInfo.OnSelected = func(id widget.TableCellID) {
		selectedRow = id.Row
		selectedColumn = id.Col
	}

	buttonBrowser := widget.NewButton("Browser", func() {
		if selectedRow > 0 && selectedColumn == 0 {
			closeBrowser()
			openBrowser()
		} else {
			dialog.ShowInformation(
				"Information",
				"Select the cell that contains a valid ID.",
				window,
			)
		}
	})

	buttonDelete := widget.NewButton("Delete", func() {
		if selectedRow > 0 && selectedColumn == 0 {
			parameter := parameters[selectedRow]
			dialog.ShowConfirm(
				"Confirm",
				fmt.Sprintf("Delete the row with ID %s ?", parameter.ID),
				func(b bool) {
					if b {
						parameter.Delete()
						saveParameters()
						if selectedRow >= len(parameters) && len(parameters) > 0 {
							selectedRow = len(parameters) - 1
						}
						tableInfo.Refresh()
						if len(parameters) > 0 {
							tableInfo.Select(widget.TableCellID{Row: selectedRow, Col: 0})
						}
					}
				},
				window,
			)
		} else {
			dialog.ShowInformation(
				"Information",
				"Select the cell that contains a valid ID.",
				window,
			)
		}
	})

	buttonEdit := widget.NewButton("Edit", func() {
		if selectedRow > 0 && selectedColumn == 0 {
			parameter := parameters[selectedRow]
			description := widget.NewEntry()
			description.SetText(parameter.Description)
			executableFile := widget.NewEntry()
			executableFile.SetText(parameter.ExecutableFile)
			initialHTMLFile := widget.NewEntry()
			initialHTMLFile.SetText(parameter.InitialHTMLFile)
			argument01 := widget.NewEntry()
			argument01.SetText(parameter.Argument01)
			argument02 := widget.NewEntry()
			argument02.SetText(parameter.Argument02)
			argument03 := widget.NewEntry()
			argument03.SetText(parameter.Argument03)
			argument04 := widget.NewEntry()
			argument04.SetText(parameter.Argument04)
			argument05 := widget.NewEntry()
			argument05.SetText(parameter.Argument05)
			executableFileContainer := container.NewBorder(nil, nil, nil, widget.NewButtonWithIcon("", theme.FileIcon(), func() {
				fileDialog(executableFile)
			}), executableFile)
			initialHTMLFileContainer := container.NewBorder(nil, nil, nil, widget.NewButtonWithIcon("", theme.FileIcon(), func() {
				fileDialog(initialHTMLFile)
			}), initialHTMLFile)
			buttons01 := container.NewHBox(widget.NewButtonWithIcon("", theme.FileIcon(), func() {
				fileDialog(argument01)
			}), widget.NewButtonWithIcon("", theme.FolderOpenIcon(), func() {
				dirDialog(argument01)
			}))
			argument01Container := container.NewBorder(nil, nil, nil, buttons01, argument01)
			buttons02 := container.NewHBox(widget.NewButtonWithIcon("", theme.FileIcon(), func() {
				fileDialog(argument02)
			}), widget.NewButtonWithIcon("", theme.FolderOpenIcon(), func() {
				dirDialog(argument02)
			}))
			argument02Container := container.NewBorder(nil, nil, nil, buttons02, argument02)
			buttons03 := container.NewHBox(widget.NewButtonWithIcon("", theme.FileIcon(), func() {
				fileDialog(argument03)
			}), widget.NewButtonWithIcon("", theme.FolderOpenIcon(), func() {
				dirDialog(argument03)
			}))
			argument03Container := container.NewBorder(nil, nil, nil, buttons03, argument03)
			buttons04 := container.NewHBox(widget.NewButtonWithIcon("", theme.FileIcon(), func() {
				fileDialog(argument04)
			}), widget.NewButtonWithIcon("", theme.FolderOpenIcon(), func() {
				dirDialog(argument04)
			}))
			argument04Container := container.NewBorder(nil, nil, nil, buttons04, argument04)
			buttons05 := container.NewHBox(widget.NewButtonWithIcon("", theme.FileIcon(), func() {
				fileDialog(argument05)
			}), widget.NewButtonWithIcon("", theme.FolderOpenIcon(), func() {
				dirDialog(argument05)
			}))
			argument05Container := container.NewBorder(nil, nil, nil, buttons05, argument05)
			items := []*widget.FormItem{
				widget.NewFormItem("Description:", description),
				widget.NewFormItem("Executable file:", executableFileContainer),
				widget.NewFormItem("Initial HTML file:", initialHTMLFileContainer),
				widget.NewFormItem("Argument 01:", argument01Container),
				widget.NewFormItem("Argument 02:", argument02Container),
				widget.NewFormItem("Argument 03:", argument03Container),
				widget.NewFormItem("Argument 04:", argument04Container),
				widget.NewFormItem("Argument 05:", argument05Container),
			}
			d := dialog.NewForm(fmt.Sprintf("Edit parameter %s", parameter.ID), "Ok", "Cancel", items, func(b bool) {
				if b {
					parameter := &Parameter{
						ID:              parameter.ID,
						Description:     strings.Trim(description.Text, " "),
						ExecutableFile:  strings.Trim(executableFile.Text, " "),
						InitialHTMLFile: strings.Trim(initialHTMLFile.Text, " "),
						Argument01:      strings.Trim(argument01.Text, " "),
						Argument02:      strings.Trim(argument02.Text, " "),
						Argument03:      strings.Trim(argument03.Text, " "),
						Argument04:      strings.Trim(argument04.Text, " "),
						Argument05:      strings.Trim(argument05.Text, " "),
					}
					parameter.Update()
					saveParameters()
					tableInfo.Refresh()
					tableInfo.Select(widget.TableCellID{Row: selectedRow, Col: 0})
				}
			},
				window,
			)
			d.Resize(fyne.NewSize(800, 400))
			d.Show()
		} else {
			dialog.ShowInformation(
				"Information",
				"Select the cell that contains a valid ID.",
				window,
			)
		}
	})

	buttonNew := widget.NewButton("New", func() {
		description := widget.NewEntry()
		executableFile := widget.NewEntry()
		initialHTMLFile := widget.NewEntry()
		argument01 := widget.NewEntry()
		argument02 := widget.NewEntry()
		argument03 := widget.NewEntry()
		argument04 := widget.NewEntry()
		argument05 := widget.NewEntry()
		executableFileContainer := container.NewBorder(nil, nil, nil, widget.NewButtonWithIcon("", theme.FileIcon(), func() {
			fileDialog(executableFile)
		}), executableFile)
		initialHTMLFileContainer := container.NewBorder(nil, nil, nil, widget.NewButtonWithIcon("", theme.FileIcon(), func() {
			fileDialog(initialHTMLFile)
		}), initialHTMLFile)
		buttons01 := container.NewHBox(widget.NewButtonWithIcon("", theme.FileIcon(), func() {
			fileDialog(argument01)
		}), widget.NewButtonWithIcon("", theme.FolderOpenIcon(), func() {
			dirDialog(argument01)
		}))
		argument01Container := container.NewBorder(nil, nil, nil, buttons01, argument01)
		buttons02 := container.NewHBox(widget.NewButtonWithIcon("", theme.FileIcon(), func() {
			fileDialog(argument02)
		}), widget.NewButtonWithIcon("", theme.FolderOpenIcon(), func() {
			dirDialog(argument02)
		}))
		argument02Container := container.NewBorder(nil, nil, nil, buttons02, argument02)
		buttons03 := container.NewHBox(widget.NewButtonWithIcon("", theme.FileIcon(), func() {
			fileDialog(argument03)
		}), widget.NewButtonWithIcon("", theme.FolderOpenIcon(), func() {
			dirDialog(argument03)
		}))
		argument03Container := container.NewBorder(nil, nil, nil, buttons03, argument03)
		buttons04 := container.NewHBox(widget.NewButtonWithIcon("", theme.FileIcon(), func() {
			fileDialog(argument04)
		}), widget.NewButtonWithIcon("", theme.FolderOpenIcon(), func() {
			dirDialog(argument04)
		}))
		argument04Container := container.NewBorder(nil, nil, nil, buttons04, argument04)
		buttons05 := container.NewHBox(widget.NewButtonWithIcon("", theme.FileIcon(), func() {
			fileDialog(argument05)
		}), widget.NewButtonWithIcon("", theme.FolderOpenIcon(), func() {
			dirDialog(argument05)
		}))
		argument05Container := container.NewBorder(nil, nil, nil, buttons05, argument05)
		items := []*widget.FormItem{
			widget.NewFormItem("Description:", description),
			widget.NewFormItem("Executable file:", executableFileContainer),
			widget.NewFormItem("Initial HTML file:", initialHTMLFileContainer),
			widget.NewFormItem("Argument 01:", argument01Container),
			widget.NewFormItem("Argument 02:", argument02Container),
			widget.NewFormItem("Argument 03:", argument03Container),
			widget.NewFormItem("Argument 04:", argument04Container),
			widget.NewFormItem("Argument 05:", argument05Container),
		}
		d := dialog.NewForm("New parameter", "Ok", "Cancel", items, func(b bool) {
			if b {
				id, err := strconv.Atoi(parameters[len(parameters)-1].ID)
				if err != nil {
					id = 0
				}
				id++
				parameter := &Parameter{
					ID:              strconv.Itoa(id),
					Description:     strings.Trim(description.Text, " "),
					ExecutableFile:  strings.Trim(executableFile.Text, " "),
					InitialHTMLFile: strings.Trim(initialHTMLFile.Text, " "),
					Argument01:      strings.Trim(argument01.Text, " "),
					Argument02:      strings.Trim(argument02.Text, " "),
					Argument03:      strings.Trim(argument03.Text, " "),
					Argument04:      strings.Trim(argument04.Text, " "),
					Argument05:      strings.Trim(argument05.Text, " "),
				}
				parameter.Create()
				saveParameters()
				row := len(parameters) - 1
				tableInfo.SetRowHeight(row, 80)
				tableInfo.Select(widget.TableCellID{Row: row, Col: 0})
			}
		},
			window,
		)
		d.Resize(fyne.NewSize(800, 400))
		d.Show()
	})

	buttonsLayout := container.NewHBox(layout.NewSpacer(), buttonNew, buttonEdit, buttonDelete, buttonBrowser)
	finalLayout := container.NewBorder(nil, buttonsLayout, nil, nil, tableInfo)
	window.SetContent(finalLayout)
	window.Resize(fyne.NewSize(1000, 500))
	window.CenterOnScreen()
	window.SetOnClosed(func() {
		closeBrowser()
	})
	window.ShowAndRun()
}

func openBrowser() {
	var err error
	browser, err = lorca.New(
		fmt.Sprintf("http://%s:%d", *host, *port),
		filepath.Join(basePath(), "temporary-folder-for-browser"),
		800,
		1000,
		"--disable-application-cache",
		"--disable-extensions",
		"--disable-infobars",
		"--disable-plugins",
		"--disable-web-security",
		"--disk-cache-size=1",
		"--excludeSwitches:enable-automation",
		"--media-cache-size=1",
		"--no-sandbox",
		"--remote-allow-origins=*",
		"--safebrowsing.enabled=false",
		"--safebrowsing_for_trusted_sources_enabled=false",
		"--window-position=0,0",
	)
	if err != nil {
		if strings.ToLower(err.Error()) == "eof" {
			dialog.ShowInformation(
				"Information",
				fmt.Sprintf(
					"%s",
					"A browser is already open. It must be closed.",
				),
				window,
			)
			return
		}
		dialog.ShowError(err, window)
	}
}

/*
func openBrowser(url string) error {
	switch runtime.GOOS {
	case "windows":
		cmd = exec.Command("rundll32", "url.dll,FileProtocolHandler", url)
	case "darwin":
		cmd = exec.Command("open", url)
	case "linux":
		cmd = exec.Command("xdg-open", url)
	default:
		return fmt.Errorf("Unsupported operating system: %s", runtime.GOOS)
	}
	err := cmd.Start()
	if err != nil {
		return err
	}
	return nil
}
*/

func saveParameters() {
	filename := filepath.Join(basePath(), "data", "parameters.json")
	byteValue, err := json.MarshalIndent(parameters, "", "\t")
	if err != nil {
		log.Fatal(err.Error())
	}
	err = os.MkdirAll(filepath.Dir(filename), 0755)
	if err != nil {
		log.Fatal(err.Error())
	}
	err = os.WriteFile(filename, byteValue, 0644)
	if err != nil {
		log.Fatal(err.Error())
	}
}
