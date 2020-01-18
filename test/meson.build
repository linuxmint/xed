test_cases = [
    'testFileMenu.py',
    'testHelpMenu.py',
    'testToolbar.py'
]

foreach case : test_cases
    test_script = find_program(case)

    test(
        case,
        test_script,
    	args: [xed.full_path()],
        is_parallel: false,
        depends: xed,
        workdir: join_paths(prefix, bindir),
        timeout: 120,
    )
endforeach
