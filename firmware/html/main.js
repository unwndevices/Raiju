// must name info box with info-id
function showInfo(id) {
    var btn = document.getElementById(id)
    var x = document.getElementById(`info-${id}`)
    if (x.classList.contains('hide')) {
        x.classList.remove('hide')
        btn.classList.add('open')
        btn.classList.remove('button-hover')
    } else {
        x.classList.add('hide')
        btn.classList.remove('open')
        btn.classList.add('button-hover')
    }
}

function getFile() {
    var json
    function handleFileSelect(evt) {
        var files = evt.target.files // FileList object
        // files is a FileList of File objects. List some properties.
    }
    if (document.getElementById('upload-data')) {
        document.addEventListener('change', handleFileSelect, false)
    }
    function handleFileSelect(evt) {
        var files = evt.target.files // FileList object

        // files is a FileList of File objects. List some properties.
        var output = []
        for (var i = 0, f; (f = files[i]); i++) {
            var reader = new FileReader()

            // Closure to capture the file information.
            reader.onload = (function (theFile) {
                return function (e) {
                    try {
                        json = JSON.parse(e.target.result)
                        var formData = JSON.stringify(json)
                        // Send data as a PUT request
                        $.ajax({
                            url: './calibration.json',
                            type: 'PUT',
                            data: formData,
                            contentType: 'application/json; charset=utf-8',
                            dataType: 'json',
                            success: function (data) {
                                console.log(formData)
                                document.getElementById('status').innerHTML =
                                    'Settings Updated'
                            },
                        })
                    } catch (ex) {}
                }
            })(f)
            reader.readAsText(f)
        }
    }

    if (document.getElementById('upload-data')) {
        document.addEventListener('change', handleFileSelect, false)
    }
}

$(document).ready(function () {
    getFile()
    // Load JSON function
    $.ajax({
        url: $('#data-form').attr('action'),
        success: function (data) {
            $.each(data, function (key, value, data) {
                if (Array.isArray(value)) {
                    for (index = 0; index < value.length; index++) {
                        var name = `${key}[${index}]`
                        var input = document.getElementsByName(name)
                        if (input.length > 0) {
                            var dataType = input[0].getAttribute('data-type')
                            if (dataType == 'boolean') {
                                $(input[0]).prop('checked', value[index])
                            } else if (dataType == 'toggle') {
                                $(input[1]).prop('checked', value[index])
                            } else {
                                $(input[0]).val(value[index])
                            }
                        }
                    }
                } else {
                    var name = key
                    var input = document.getElementsByName(name)
                    if (input.length > 0) {
                        var dataType = input[0].getAttribute('data-type')
                        if (dataType == 'boolean') {
                            $(input[0]).prop('checked', value)
                        } else if (dataType == 'toggle') {
                            $(input[1]).prop('checked', value)
                        } else {
                            $(input[0]).val(value)
                        }
                        if (input[0].hasAttribute('oninput')) {
                            input[0].oninput()
                        }
                    }
                }
            })
        },
    })

    // Submit FORM function
    $('form').on('submit', function (e) {
        document.getElementById('status').innerHTML = ''
        var obj = $('form').serializeJSON({ useIntKeysAsArrayIndex: true })
        // if ($('form').hasClass('calibration-form')) {
        //   var index = { calibrated: true }
        // Object.assign(obj, index)
        //}
        var formData = JSON.stringify(obj)

        // Send data as a PUT request
        $.ajax({
            url: $(this).attr('action'),
            type: 'PUT',
            data: formData,
            contentType: 'application/json; charset=utf-8',
            dataType: 'json',
            success: function (data) {
                console.log(formData)
                document.getElementById('status').innerHTML = 'Settings Updated'
            },
        })

        return false
    })

    // Download calibration JSON
    $('#calibration-download').click(function (e) {
        e.preventDefault() //stop the browser from following
        var request = new XMLHttpRequest()
        request.open('GET', './calibration.json', false)
        request.send(null)
        var calibration = JSON.parse(request.responseText)
        // Convert object to Blob
        const blobConfig = new Blob([JSON.stringify(calibration)], {
            type: 'text/json;charset=utf-8',
        })

        // Convert Blob to URL
        const blobUrl = URL.createObjectURL(blobConfig)

        // Create an a element with blobl URL
        const anchor = document.createElement('a')
        anchor.href = blobUrl
        anchor.target = '_blank'
        anchor.download = 'raiju_calibration.json'

        // Auto click on a element, trigger the file download
        anchor.click()

        // Don't forget ;)
        URL.revokeObjectURL(blobUrl)
    })

    if (window.location.pathname === '/calibration.html') {
        setInterval(function () {
            $.get('/voct', function (data) {
                $('#analog-value').text(data)
            })
        }, 2000) // Adjust this value to change the refresh rate. This is currently set to 1000ms or 1 second.
    }
})
