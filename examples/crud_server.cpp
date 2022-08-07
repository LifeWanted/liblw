/**
 * An example REST server defining Book and Author resources. While this example
 * defines both in one file, best practice would be for each of the `*Handler`
 * classes to be defined in separate compilation units.
 *
 * If `--tls-cert-path` and `--tls-key-path` are provided, then an HTTPS server
 * will be created. Otherwise an HTTP server (no encryption) is started.
 */

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>

#include "lw/co/generator.h"
#include "lw/co/future.h"
#include "lw/flags/flags.h"
#include "lw/io/serializer/serializer.h"
// #include "lw/mime/form_urlencoded.h"
#include "lw/mime/json.h"
#include "lw/net/server_resource.h"
#include "lw/http/rest/rest_handler.h"
#include "lw/http/rest/rest_router.h"

LW_FLAG(
  unsigned short, port, 8080,
  "Port for the server to listen on for HTTP."
);
LW_FLAG(
  std::string, tls_cert_path, "",
  "Path to the TLS certificate."
);
LW_FLAG(
  std::string, tls_key_path, "",
  "Path to the TLS private key."
);

namespace {

class Author;
class Book;
class NewBook;

/**
 * A fake database "connection." An instance of this class is created for every
 * request to handlers which list it as a dependency.
 */
class Database: public lw::ServerResource<> {
public:
  Database() = default;

  Author getAuthor(std::uint64_t id) const;
  std::uint64_t addAuthor(Author author);
  void updateAuthor(const Author& author);
  void removeAuthor(std::uint64_t id);

  Book getBook(std::uint64_t id) const;
  std::uint64_t addBook(NewBook book);
  void updateBook(const Book& book);
  void removeBook(std::uint64_t id);

  std::vector<Author> getAuthors(std::size_t offset, std::size_t limit) const;
  std::size_t countAuthors() const { return _db.authors.size(); }

  std::vector<Book> getBooksByAuthor(
    std::uint64_t id,
    std::size_t offset,
    std::size_t limit
  ) const;
  std::size_t countBooksByAuthor(std::uint64_t id) const;

private:
  // A fake "remote" database. This structure represents the actual data stored
  // in a remote server which the `Database` class "connects" to. This is done
  // to simplify this example, but a real one would create a connection or use
  // a connection pool to talk to the remote server.
  struct Resources {
    std::unordered_map<std::uint64_t, Author> authors;
    std::unordered_map<std::uint64_t, Book> books;
  };

  static Resources _db;
};

Database::Resources Database::_db;

LW_REGISTER_RESOURCE_FACTORY(Database, []() {
  return std::make_unique<Database>();
});

// -------------------------------------------------------------------------- //

class Author {
public:
  Author() = default;
  Author(const Author&) = default;
  Author(Author&&) = default;
  ~Author() = default;
  Author& operator=(const Author&) = default;
  Author& operator=(Author&&) = default;

  Author(std::uint64_t id, std::string name): id{id}, name{std::move(name)} {}

  std::uint64_t id = 0;
  std::string name;
};

// -------------------------------------------------------------------------- //

class Book {
public:
  Book() = default;
  Book(const Book&) = default;
  Book(Book&&) = default;
  ~Book() = default;
  Book& operator=(const Book&) = default;
  Book& operator=(Book&&) = default;

  Book(std::uint64_t id, std::string title, Author author):
    id{id},
    title{std::move(title)},
    author{std::move(author)}
  {}

  std::uint64_t id = 0;
  std::string title;
  Author author;
};

class NewBook {
public:
  NewBook() = default;
  NewBook(const NewBook&) = default;
  NewBook(NewBook&&) = default;
  ~NewBook() = default;
  NewBook& operator=(const NewBook&) = default;
  NewBook& operator=(NewBook&&) = default;

  NewBook(std::string title, std::uint64_t author_id):
    title{std::move(title)},
    author_id{author_id}
  {}

  std::string title;
  std::uint64_t author_id;
};

// -------------------------------------------------------------------------- //

class AuthorListHandler:
  public lw::CrudHandler<
    Author,
    std::tuple<lw::DefaultRestDependencies, Database>
  >
{
public:
  lw::co::Future<void> create() override {
    Author author = request().body_as<Author>();
    if (author.id) {
      throw InvalidArgumentRestError()
        << "Author Id can not be specified on creation.";
    }

    Database& db = get<Database>();
    std::uint64_t id = db.addAuthor(author);
    author.id = id;
    co_await response().write(author);
  }

  lw::co::Future<void> read() override {
    lw::PageRequest req = request().query_as<lw::PageRequest>();
    Database& db = get<Database>();
    std::vector<Author> authors = db.getAuthors(req.offset, req.limit);
    co_await response().write(
      lw::PageResponse<Author>{
        std::move(authors),
        req.offset,
        db.countAuthors()
      }
    );
  }
};

LW_REGISTER_REST_HANDLER(AuthorListHandler, "/authors");

// -------------------------------------------------------------------------- //

class AuthorHandler:
  public lw::CrudHandler<
    Author,
    std::tuple<lw::DefaultRestDependencies, Database>
  >
{
public:
  lw::co::Future<void> read() override {
    Author author = get_request_author_from_db();
    co_await response().write(author);
  }

  lw::co::Future<void> update() override {
    Author author = get_request_author_from_db();
    if (request().body_has("name")) {
      author.name = request().body().get<std::string>("name");
    }
    db.saveAuthor(author);
    co_await response().write(author);
  }

  lw::co::Future<void> destroy() override {
    std::uint64_t id = request().route_param_as<std::uint64_t>("id");
    if (!get<Database>().hasAuthor(id)) {
      throw lw::NotFoundRestError() << "No author with id " << id << " found.";
    }
    get<Database>().removeAuthor(id);
    co_return;
  }

private:
  Author get_request_author_from_db() {
    std::uint64_t id = request().route_param_as<std::uint64_t>("id");
    if (!get<Database>().hasAuthor(id)) {
      throw lw::NotFoundRestError() << "No author with id " << id << " found.";
    }

    return get<Database>().getAuthor(id);
  }
};

LW_REGISTER_REST_HANDLER(AuthorHandler, "/authors/:[uint]id");

// -------------------------------------------------------------------------- //

class AuthorBooksHandler:
  lw::CrudHandler<
    Book,
    std::tuple<lw::DefaultRestDependencies, Database>
  >
{
public:
  lw::co::Future<void> read() override {
    std::uint64_t id = request().route_param_as<std::uint64_t>("id");
    lw::PageRequest page = request().body_as<lw::PageRequest>();

    if (!get<Database>().hasAuthor(id)) {
      throw lw::NotFoundRestError() << "No author with id " << id << " found.";
    }

    std::vector<Book> books =
      get<Database>().getBooksByAuthor(id, page.offset, page.limit);
    std::size_t book_count = get<Database>().countBooksByAuthor(id);
    co_await response().write(
      lw::PageResponse<Book>{std::move(books), page.offset, book_count}
    );
  }
};

LW_REGISTER_REST_HANDLER(AuthorBooksHandler, "/authors/:[uint]id/books");

// -------------------------------------------------------------------------- //

class BookListHandler:
  public lw::CrudHandler<
    Book,
    std::tuple<lw::DefaultRestDependencies, Database>
  >
{
public:
  lw::co::Future<void> create() override {
    NewBook book = request().body_as<NewBook>();
    if (!get<Database>().hasAuthor(book.author_id)) {
      throw lw::InvalidArgumentRestError()
        << "Author " << book.author_id << " does not exist.";
    }

    Database& db = get<Database>();
    std::uint64_t id = db.addBook(book);
    Book res{id, book.title, db.getAuthor(book.author_id)};
    co_await response().write(res);
  }
};

LW_REGISTER_REST_HANDLER(BookListHandler, "/books");

// -------------------------------------------------------------------------- //

class BookHandler:
  public lw::CrudHandler<
    Book,
    std::tuple<lw::DefaultRestDependencies, Database>
  >
{
public:
  lw::co::Future<void> read() override {
    Book book = get_request_book_from_db();
    co_await response().write(book);
  }

  lw::co::Future<void> update() override {
    Book book = get_request_book_from_db();
    if (request().body_has("name")) {
      book.name = request().body().get<std::string>("name");
    }
    if (request().body_has({"author", "id"})) {
      std::uint64_t author_id =
        request().body().get<std::uint64_t>({"author", "id"});
      if (!get<Database>().hasAuthor(author_id)) {
        throw lw::InvalidArgumentRestError()
          << "Author " << author_id << " does not exist.";
      }
      book.author = get<Database>().getAuthor(author_id);
    }
    db.saveBook(book);
    co_await response().write(book);
  }

  lw::co::Future<void> destroy() override {
    std::uint64_t id = request().route_param_as<std::uint64_t>("id");
    if (!get<Database>().hasBook(id)) {
      throw lw::NotFoundRestError() << "No book with id " << id << " found.";
    }
    get<Database>().removeBook(id);
    co_return;
  }

private:
  Book get_request_book_from_db() {
    std::uint64_t id = request().route_param_as<std::uint64_t>("id");
    if (!get<Database>().hasBook(id)) {
      throw lw::NotFoundRestError() << "No book with id " << id << " found.";
    }

    return get<Database>().getBook(id);
  }
};

LW_REGISTER_REST_HANDLER(BookHandler, "/books/:[uint]id");

// -------------------------------------------------------------------------- //

// The implementation of the example database is unimportant to the REST
// interface presented above. It is only here to make a functional example that
// stores state between requests. Realistically, this would be done using a
// database server and SQL queries.

Author Database::getAuthor(std::uint64_t id) const {
  return _db.authors[id];
}

std::uint64_t Database::addAuthor(Author author) {
  std::size_t id = _db.authors.size() + 1;
  author.id = id;
  _db.authors[id] = std::move(author);
  return id;
}

void Database::updateAuthor(const Author& author) {
  std::uint64_t id = author.id;
  _db.authors[id] = std::move(author);
}

void Database::removeAuthor(std::uint64_t id) {
  _db.authors.erase(id);
}

Book Database::getBook(std::uint64_t id) const {
  return _db.books[id];
}

std::uint64_t Database::addBook(NewBook new_book) {
  std::size_t id = _db.books.size() + 1;
  Book book{id, new_book.title, getAuthor(new_book.author_id)};
  _db.books[id] = std::move(book);
  return id;
}

void Database::updateBook(const Book& book) {
  std::uint64_t id = book.id;
  _db.books[id] = std::move(book);
}

void Database::removeBook(std::uint64_t id) {
  _db.books.erase(id);
}

std::vector<Author> Database::getAuthors(
  std::size_t offset,
  std::size_t limit
) const {
  std::vector<Author> authors;
  std::size_t counter = 0;
  for (const auto& [id, author] : _db.authors) {
    if (++counter > offset) {
      authors.push(author);
      if (authors.size() >= limit) break;
    }
  }
  return authors;
}

std::vector<Book> Database::getBooksByAuthor(
  std::uint64_t id,
  std::size_t offset,
  std::size_t limit
) const {
  std::vector<Book> books;
  std::size_t counter = 0;
  for (const auto& [id, book] : _db.books) {
    if (book.author.id == id && ++counter > offset) {
      books.push(book);
      if (books.size() >= limit) break;
    }
  }
  return books;
}

std::size_t Database::countBooksByAuthor(std::uint64_t id) const {
  std::size_t counter = 0;
  for (const auto& [id, book] : _db.books) {
    if (book.author.id == id) ++counter;
  }
  return counter;
}

// -------------------------------------------------------------------------- //

}

/**
 * Specializations of the Serialize class must be made for all the types which
 * will be serialized or deserialized.
 */
template <>
struct lw::io::Serialize<Author> {
  typedef lw::io::ObjectTag serialization_category;

  void serialize(lw::io::Serializer& serializer, const Author& author) {
    serializer.write("id", author.id);
    serializer.write("name", author.name);
  }

  Author deserialize(const lw::io::SerializedValue& value) {
    return {
      value.get<std::uint64_t>("id", 0),
      value.get<std::string_view>("name")
    };
  }
};

template <>
struct lw::io::Serialize<Book> {
  typedef lw::io::ObjectTag serialization_category;

  void serialize(lw::io::Serializer& serializer, const Book& book) {
    serializer.write("id", book.id);
    serializer.write("title", book.title);
    serializer.write("author", book.author);
  }

  Book deserialize(const lw::io::SerializedValue& value) {
    return {
      value.get<std::uint64_t>("id"),
      value.get<std::string_view>("title"),
      value.get<Author>("author")
    };
  }
};

template <>
struct lw::io::Serialize<NewBook> {
  typedef lw::io::ObjectTag serialization_category;

  NewBook deserialize(const lw::io::SerializedValue& value) {
    return {
      value.get<std::string_view>("title"),
      value.get<std::uint64_t>("author_id")
    };
  }
};

// -------------------------------------------------------------------------- //

int main(int argc, const char** argv) {
  // Routers must outlive the server, so we'll declare them outside the try.
  std::unique_ptr<lw::RestRouter> router;
  try {
    if (!lw::init(&argc, argv)) {
      return 0;
    }
    lw::net::Server server;

    // Enable HTTPS routing only if certificate and key are provided.
    lw::RestRouter::Options opts{
      .default_content_type = "application/json"
    };
    if (
      !lw::flags::tls_cert_path.value().empty() &&
      !lw::flags::tls_key_path.value().empty()
    ) {
      lw::log(lw::INFO) << "Creating HTTPS router.";
      opts.https_options = lw::HttpsRouter::Options{
        .private_key = lw::flags::tls_key_path.value(),
        .certificate = lw::flags::tls_cert_path.value()
      };
    } else {
      lw::log(lw::INFO) << "Creating HTTP router.";
    }

    router = std::make_unique<lw::RestRouter>(opts);
    router->add_serializer(lw::mime::JSONSerializer{});
    router->add_serializer(lw::mime::FormURLEncodedSerializer{});

    server.attach_router(lw::flags::port, router.get());
    server.listen();
    lw::log(lw::INFO) << "Server is listening on " << lw::flags::port;
    server.run();
  } catch (const std::runtime_error& err) {
    lw::log(lw::ERROR) << err.what();
    return -1;
  }
  return 0;
}
