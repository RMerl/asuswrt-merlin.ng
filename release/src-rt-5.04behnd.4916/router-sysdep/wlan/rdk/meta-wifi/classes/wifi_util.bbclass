def check_feature(d, file, feature):
    impl_dir = d.getVar('WLIMPL_PATH')
    search_file = os.path.join(impl_dir, file)
    try:
        bb.note(search_file)
        with open(search_file, 'r') as file:
            for line in file:
                if feature in line and not line.strip().startswith("#"):
                    return feature 
        return ""
    except FileNotFoundError:
        bb.note(f"Error: The file at '{search_file}' was not found.")
        return ""
    except Exception as e:
        return ""
