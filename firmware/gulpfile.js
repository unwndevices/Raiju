var gulp = require('gulp')
var cssnano = require('gulp-cssnano')
var terser = require('gulp-terser')
const htmlmin = require('gulp-htmlmin')
var replace = require('gulp-replace')
var gzip = require('gulp-gzip')
var removeHtmlComments = require('gulp-remove-html-comments')
var clean = require('gulp-clean')
fs = require('fs')

gulp.task('json', function (done) {
    gulp.src('html/parameters.json').pipe(gulp.dest('data'))
    gulp.src('html/calibration.json').pipe(gulp.dest('data'))
    return gulp.src('html/configuration.json').pipe(gulp.dest('data'))
    done()
})

gulp.task('css', function (done) {
    gulp.src('html/style.css').pipe(cssnano()).pipe(gulp.dest('temp'))
    gulp.src('html/update.css').pipe(cssnano()).pipe(gulp.dest('temp')) ///update
    return gulp.src('html/toggles.css').pipe(cssnano()).pipe(gulp.dest('temp'))
    done()
})

gulp.task('js', function (done) {
    gulp.src('html/jquery.serializejson.js')
        .pipe(terser())
        .pipe(gulp.dest('temp'))
    gulp.src('html/update.js').pipe(terser()).pipe(gulp.dest('temp'))
    return gulp.src('html/main.js').pipe(terser()).pipe(gulp.dest('temp'))
    done()
})

gulp.task('html', function (done) {
    gulp.src('html/calibration.html')
        .pipe(htmlmin({ collapseWhitespace: true }))
        .pipe(gulp.dest('temp'))
    gulp.src('html/update.html')
        .pipe(htmlmin({ collapseWhitespace: true }))
        .pipe(gulp.dest('temp'))
    return gulp
        .src('html/index.html')
        .pipe(htmlmin({ collapseWhitespace: true }))
        .pipe(gulp.dest('temp'))
    done()
})

gulp.task('clean', function () {
    return gulp.src('temp/*', { read: false }).pipe(clean())
})

gulp.task('compress', function (done) {
    gulp.src('temp/index.html').pipe(gzip()).pipe(gulp.dest('data'))
    gulp.src('temp/calibration.html').pipe(gzip()).pipe(gulp.dest('data'))
    gulp.src('temp/update.html').pipe(gzip()).pipe(gulp.dest('data'))
    done()
})

gulp.task('html', function () {
    gulp.src('html/calibration.html')
        .pipe(removeHtmlComments())
        .pipe(
            replace(
                /<link rel="stylesheet" href="style.css"[^>]*>/,
                function (s) {
                    var style = fs.readFileSync('temp/style.css', 'utf8')
                    return '<style>\n' + style + '\n</style>'
                }
            )
        )
        .pipe(
            replace(/<script src="main.js"\><\/script\>/, function (s) {
                var script = fs.readFileSync('temp/main.js', 'utf8')
                return '<script>\n' + script + '\n</script>'
            })
        )
        .pipe(
            replace(
                /<link rel="stylesheet" href="toggles.css"[^>]*>/,
                function (s) {
                    var style = fs.readFileSync('temp/toggles.css', 'utf8')
                    return '<style>\n' + style + '\n</style>'
                }
            )
        )
        .pipe(
            replace(
                /<script src="jquery.serializejson.js"\><\/script\>/,
                function (s) {
                    var script = fs.readFileSync(
                        'temp/jquery.serializejson.js',
                        'utf8'
                    )
                    return '<script>\n' + script + '\n</script>'
                }
            )
        )
        .pipe(
            replace(/<script src="jquery.min.js"\><\/script\>/, function (s) {
                var script = fs.readFileSync('html/jquery.min.js', 'utf8')
                return '<script>\n' + script + '\n</script>'
            })
        )
        .pipe(htmlmin({ collapseWhitespace: true }))
        .pipe(gulp.dest('temp'))

    gulp.src('html/update.html')
        .pipe(removeHtmlComments())
        .pipe(
            replace(
                /<link rel="stylesheet" href="update.css"[^>]*>/,
                function (s) {
                    var style = fs.readFileSync('temp/update.css', 'utf8')
                    return '<style>\n' + style + '\n</style>'
                }
            )
        )
        .pipe(
            replace(/<script src="update.js"\><\/script\>/, function (s) {
                var script = fs.readFileSync('temp/update.js', 'utf8')
                return '<script>\n' + script + '\n</script>'
            })
        )

        .pipe(
            replace(/<script src="jquery.min.js"\><\/script\>/, function (s) {
                var script = fs.readFileSync('html/jquery.min.js', 'utf8')
                return '<script>\n' + script + '\n</script>'
            })
        )
        .pipe(htmlmin({ collapseWhitespace: true }))
        .pipe(gulp.dest('temp'))
    return gulp
        .src('html/index.html')
        .pipe(removeHtmlComments())
        .pipe(
            replace(
                /<link rel="stylesheet" href="style.css"[^>]*>/,
                function (s) {
                    var style = fs.readFileSync('temp/style.css', 'utf8')
                    return '<style>\n' + style + '\n</style>'
                }
            )
        )
        .pipe(
            replace(
                /<link rel="stylesheet" href="toggles.css"[^>]*>/,
                function (s) {
                    var style = fs.readFileSync('temp/toggles.css', 'utf8')
                    return '<style>\n' + style + '\n</style>'
                }
            )
        )
        .pipe(
            replace(/<script src="main.js"\><\/script\>/, function (s) {
                var script = fs.readFileSync('temp/main.js', 'utf8')
                return '<script>\n' + script + '\n</script>'
            })
        )
        .pipe(
            replace(
                /<script src="jquery.serializejson.js"\><\/script\>/,
                function (s) {
                    var script = fs.readFileSync(
                        'temp/jquery.serializejson.js',
                        'utf8'
                    )
                    return '<script>\n' + script + '\n</script>'
                }
            )
        )
        .pipe(
            replace(/<script src="jquery.min.js"\><\/script\>/, function (s) {
                var script = fs.readFileSync('html/jquery.min.js', 'utf8')
                return '<script>\n' + script + '\n</script>'
            })
        )
        .pipe(htmlmin({ collapseWhitespace: true }))
        .pipe(gulp.dest('temp'))
})

gulp.task(
    'default',
    gulp.series('json', 'css', 'js', 'html', 'compress', 'clean')
)

// -----------------------------------------------------------------------------
// PlatformIO support
// -----------------------------------------------------------------------------

const spawn = require('child_process').spawn
const argv = require('yargs').argv

var platformio = function (target) {
    var args = ['run']
    if ('e' in argv) {
        args.push('-e')
        args.push(argv.e)
    }
    if ('p' in argv) {
        args.push('--upload-port')
        args.push(argv.p)
    }
    if (target) {
        args.push('-t')
        args.push(target)
    }
    const cmd = spawn('platformio', args)
    cmd.stdout.on('data', function (data) {
        console.log(data.toString().trim())
    })
    cmd.stderr.on('data', function (data) {
        console.log(data.toString().trim())
    })
}

gulp.task('uploadfs', gulp.series('default'), function () {
    platformio('uploadfs')
})
gulp.task('upload', function () {
    platformio('upload')
})
gulp.task('run', function () {
    platformio(false)
})
