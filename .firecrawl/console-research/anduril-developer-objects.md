For AI agents: a documentation index is available at the root level at /llms.txt and /llms-full.txt. Append /llms.txt to any URL for a page-level index, or .md for the markdown version of any page.

In Lattice, an **object** is a data model that represents a single file or a piece of data. This page explains how to upload an object to Lattice
and associate it with an entity using the Lattice SDK.

In the following steps, you use an image, and a CSV file. Lattice, however, supports all binary data, letting you use objects
to implement a variety of use-cases. For example, consider a scenario where an image sensor integrated with Lattice detects
a track image and uploads an image of it to Lattice. After the file is uploaded, any device, app, or operator in Lattice
can access the image associated with the track.

To implement this use-case, you use the following API operations:

- [**`UploadObject`**](https://developer.anduril.com/reference/rest/objects/upload-object) — Uploads an object using multiform data.

- [**`PutEntityOverride`**](https://developer.anduril.com/reference/rest/entities/override-entity) — Override the entity
and update the entity’s [`media`](https://developer.anduril.com/reference/rest/entities/publish-entity#request.body.media) component.











Entity overrides let any integration update certain components in an entity,
even if the entity was initially published by another data source. This means that
anyone connected to Lattice can update the media associated with the entity, giving you
more flexibility.


## Before you begin

- To configure your app to upload objects, [set up](https://developer.anduril.com/guides/getting-started/install) your Lattice environment.
- See an overview of the [Object schema and the Object Store API](https://developer.anduril.com/guides/objects/overview) in Lattice.

##### gRPC authentication

If you are using gRPC with client credentials, [set up the token refresh module](https://developer.anduril.com/guides/getting-started/authenticate#using-grpc) before running the examples on this page.

## Upload an object

In Lattice, an **object** is a data model that represents a single file or a piece of data. To upload an object to Lattice, do the following:

[1](https://developer.anduril.com/guides/objects/upload#choose-an-image)

### Choose an image

Choose an image to upload to Lattice, and save it locally.

[2](https://developer.anduril.com/guides/objects/upload#upload-the-image-to-lattice)

### Upload the image to Lattice

Run the following code from the same folder where you saved the image.
To upload, use [`UploadObject`](https://developer.anduril.com/reference/rest/objects/upload-object) and replace the following:

- `file_path`: the path to the file you want to upload to Lattice.

GoJavaPythonTypescript

```
package main

import (
	"bytes"
	"context"
	"fmt"
	"net/http"
	"os"
	"path/filepath"

	"github.com/anduril/lattice-sdk-go/v4/client"
	"github.com/anduril/lattice-sdk-go/v4/option"
)

func main() {
	// Get environment variables.
	latticeEndpoint := os.Getenv("LATTICE_ENDPOINT")
	clientSecret := os.Getenv("LATTICE_CLIENT_SECRET")
	clientId := os.Getenv("LATTICE_CLIENT_ID")

	// Remove sandboxesToken from the following statements if you are not developing on Sandboxes.
	sandboxesToken := os.Getenv("SANDBOXES_TOKEN")

	// Check required environment variables.
	if latticeEndpoint == "" || clientId == "" || clientSecret == "" || sandboxesToken == "" {
		fmt.Println("Missing required environment variables")
		os.Exit(1)
	}

	// Initialize headers for sandbox authorization.
	headers := http.Header{}
	headers.Add("Anduril-Sandbox-Authorization", fmt.Sprintf("Bearer %s", sandboxesToken))

	// Create the client
	c := client.NewClient(
		option.WithClientCredentials(clientId, clientSecret),
		option.WithBaseURL(fmt.Sprintf("https://%s", latticeEndpoint)),
		option.WithHTTPHeader(headers),
	)

	// Set the file path.
	filePath := "<FILE_PATH>"

	// Get the file name.
	fileName := filepath.Base(filePath)

	// Read the file contents into a byte slice.
	fileContent, err := os.ReadFile(filePath)
	if err != nil {
		fmt.Printf("Error reading file: %v\n", err)
		os.Exit(1)
	}

	// Create context for the request.
	ctx := context.Background()

	// Upload the file to Lattice.
	response, err := c.Objects.UploadObject(ctx, fileName, bytes.NewReader(fileContent))
	if err != nil {
		fmt.Printf("Error uploading object: %v\n", err)
		os.Exit(1)
	}

	// Get the content identifier path.
	path := response.ContentIdentifier.Path

	fmt.Printf("Object path: /api/v1/objects/%s\n", path)
}
```

An object path must only contain the following characters: `A-Z`,
`a-z`, `0-9`, `.``_`, `-`.

[3](https://developer.anduril.com/guides/objects/upload#verify-the-response)

### Verify the response

If successful, you’ll see the following output.

```
$ python upload.py
INFO - HTTP Request: POST https://lattice-50802.env.sandboxes.developer.anduril.com/api/v1/objects/test-1.test.jpg "HTTP/1.1 200 OK"
INFO - Object path: /api/v1/objects/cessna.jpg
```

Note the object path: `/api/v1/objects/cessna.jpg`.
In the following section, you’ll override an entity to associate it with this object using its unique object path.
This lets Lattice know to display the image in the entity panel as a thumbnail.

## Link an object

To associate an image with an entity, do the following:

[1](https://developer.anduril.com/guides/objects/upload#override-the-entity)

### Override the entity

Use the
[`OverrideEntity`](https://developer.anduril.com/reference/rest/entities/override-entity) operation to override the entity’s
[`media`](https://developer.anduril.com/reference/rest/entities/publish-entity#request.body.media) component and replace `MediaItem`.
To implement a thumbnail, set [`MediaItem.type`](https://developer.anduril.com/reference/rest/entities/publish-entity#request.body.media.media.type) to `MEDIA_TYPE_IMAGE`.
This lets Lattice know to process the binary object as an image.

Replace the temporary values with your information, then run the code:

- `object_path`: The unique path of the object in Lattice. For this example, use `/api/v1/objects/test-1.test.jpg`.
- `entity_id`: The ID of the entity you want to associate with the object. If you are developing
in Sandboxes, enter `adsbEntity` to use an existing simulated entity.

GogRPC/GogRPC/RustJavaPythonTypescript

```
package main

import (
	"context"
	"fmt"
	"net/http"
	"os"
	"time"

	Lattice "github.com/anduril/lattice-sdk-go/v4"
	"github.com/anduril/lattice-sdk-go/v4/client"
	"github.com/anduril/lattice-sdk-go/v4/option"
)

func main() {
	// Get environment variables
	latticeEndpoint := os.Getenv("LATTICE_ENDPOINT")
	clientSecret := os.Getenv("LATTICE_CLIENT_SECRET")
	clientId := os.Getenv("LATTICE_CLIENT_ID")

	// Remove sandboxesToken from the following statements if you are not developing on Sandboxes.
	sandboxesToken := os.Getenv("SANDBOXES_TOKEN")

	// Check required environment variables
	if latticeEndpoint == "" || clientId == "" || clientSecret == "" || sandboxesToken == "" {
		fmt.Println("Missing required environment variables")
		os.Exit(1)
	}

	// Initialize headers for sandbox authorization
	headers := http.Header{}
	headers.Add("Anduril-Sandbox-Authorization", fmt.Sprintf("Bearer %s", sandboxesToken))

	// Create the client
	LatticeClient := client.NewClient(
		option.WithClientCredentials(clientId, clientSecret),
		option.WithBaseURL(fmt.Sprintf("https://%s", latticeEndpoint)),
		option.WithHTTPHeader(headers),
	)

	// Set the path to the object and entity ID
	objectPath := "/api/v1/objects/<objectPath>"
	entityID := "adsbEntity"

	// Create context for the request
	ctx := context.Background()

	mediaItem := &Lattice.MediaItem{
		Type:         Lattice.MediaItemTypeMediaTypeImage.Ptr(),
		RelativePath: &objectPath,
	}

	// Create media object with the media item
	mediaItems := []*Lattice.MediaItem{mediaItem}
	media := Lattice.Media{
		Media: mediaItems,
	}

	// Create a provenance object
	latestTimestamp := time.Now().UTC()

	// Create an entity with the media
	entity := Lattice.EntityOverride{
		EntityID:  entityID,
		FieldPath: "media.media",
		Entity: &Lattice.Entity{
			EntityID: &entityID,
			Media:    &media,
		},
		Provenance: &Lattice.Provenance{
			IntegrationName:  Lattice.String("your_integration_name"),
			DataType:         Lattice.String("test_data"),
			SourceUpdateTime: Lattice.Time(latestTimestamp),
		},
	}

	// Override the entity's media
	_, err := LatticeClient.Entities.OverrideEntity(
		ctx,
		&entity,
	)

	// Handle errors
	if err != nil {
		fmt.Printf("Exception: %v\n", err)
	} else {
		fmt.Printf("Successfully added media to entity: %s\n", entityID)
	}
}
```

[2](https://developer.anduril.com/guides/objects/upload#verify-the-response-1)

### Verify the response

If successful, you’ll see the following output:

```
$ python override_media_add.py
INFO - HTTP Request: PUT https://your_lattice_endpoint.env.sandboxes.developer.anduril.com/api/v1/entities/adsbEntity/override/media.media "HTTP/1.1 200 OK"
```

## Check the thumbnail

To see the track thumbnail, open the your environment Lattice UI, and do the following:

[1](https://developer.anduril.com/guides/objects/upload#open-the-track-panel)

### Open the track panel

Open the track panel from the left hand sidebar.
Enter the `aliases.name` of the surface vessel in the search bar.

If you are developing on Sandboxes, enter the following name: N113PF.

![Shows the panel where you can search for the surface vessel.](https://developer.anduril.com/guides/objects/_files/anduril.docs.buildwithfern.com/58c183e84a49edc4eb2b0c8a5db18eb71e36c6ebdfd68091cd2950a16858f5dc/assets/images/screenshots/lattice-ui-track-panel-n113pf.png)

[2](https://developer.anduril.com/guides/objects/upload#verify-the-thumbnail)

### Verify the thumbnail

Select the thumbnail associated with the track to expand the image:

![Shows the uploaded track thumbnail.](https://developer.anduril.com/guides/objects/_files/anduril.docs.buildwithfern.com/809906483da2a3d55750d8faf43dcf0d27019d45e3defc038755686b2a8579a5/assets/images/screenshots/lattice-ui-track-panel-n113pf-thumbnail.png)

## Link multiple objects

In some cases, you might associate more than one object with an entity. An airplane, for instance, might link to an image
to display a thumbnail, and a CSV that contains additional metadata, such as the airplane’s flight manifest.

To do this, get the existing media linked with the entity, then override the entity’s `media` component
to append a new `MediaItem` to the list:

[1](https://developer.anduril.com/guides/objects/upload#create-a-manifest)

### Create a manifest

Copy and paste the the example into a new CSV file, named `manifest.csv`:

manifest.csv

```
FlightID,FlightName,PilotName,DepartureAirport,ArrivalAirport,DepartureTime,ArrivalTime
67890,Cessna Flight,Jane Pilot,John Wayne Airport,San Francisco International,2023-10-01 09:00,2023-10-01 11:00

PassengerID,PassengerName,Weight
P001,Thomas Pynchon,180
P002,Anthony Burgess,150

CargoID,Description,Weight
001,Luggage,50
002,Equipment,30
```

[2](https://developer.anduril.com/guides/objects/upload#upload-the-file)

### Upload the file

Run the following code from the same folder where you saved `manifest.csv`.
To upload, use [`UploadObject`](https://developer.anduril.com/reference/rest/objects/upload-object) and replace the following:

- `file_path`: The path to the file you want to upload to Lattice,
in this case, `./manifest.csv`

GoJavaPythonTypescript

```
package main

import (
	"bytes"
	"context"
	"fmt"
	"net/http"
	"os"
	"path/filepath"

	"github.com/anduril/lattice-sdk-go/v4/client"
	"github.com/anduril/lattice-sdk-go/v4/option"
)

func main() {
	// Get environment variables.
	latticeEndpoint := os.Getenv("LATTICE_ENDPOINT")
	clientSecret := os.Getenv("LATTICE_CLIENT_SECRET")
	clientId := os.Getenv("LATTICE_CLIENT_ID")

	// Remove sandboxesToken from the following statements if you are not developing on Sandboxes.
	sandboxesToken := os.Getenv("SANDBOXES_TOKEN")

	// Check required environment variables.
	if latticeEndpoint == "" || clientId == "" || clientSecret == "" || sandboxesToken == "" {
		fmt.Println("Missing required environment variables")
		os.Exit(1)
	}

	// Initialize headers for sandbox authorization.
	headers := http.Header{}
	headers.Add("Anduril-Sandbox-Authorization", fmt.Sprintf("Bearer %s", sandboxesToken))

	// Create the client
	c := client.NewClient(
		option.WithClientCredentials(clientId, clientSecret),
		option.WithBaseURL(fmt.Sprintf("https://%s", latticeEndpoint)),
		option.WithHTTPHeader(headers),
	)

	// Set the file path.
	filePath := "<FILE_PATH>"

	// Get the file name.
	fileName := filepath.Base(filePath)

	// Read the file contents into a byte slice.
	fileContent, err := os.ReadFile(filePath)
	if err != nil {
		fmt.Printf("Error reading file: %v\n", err)
		os.Exit(1)
	}

	// Create context for the request.
	ctx := context.Background()

	// Upload the file to Lattice.
	response, err := c.Objects.UploadObject(ctx, fileName, bytes.NewReader(fileContent))
	if err != nil {
		fmt.Printf("Error uploading object: %v\n", err)
		os.Exit(1)
	}

	// Get the content identifier path.
	path := response.ContentIdentifier.Path

	fmt.Printf("Object path: /api/v1/objects/%s\n", path)
}
```

[3](https://developer.anduril.com/guides/objects/upload#override-the-entity-1)

### Override the entity

Use [`OverrideEntity`](https://developer.anduril.com/reference/rest/entities/override-entity) to modify the entity’s
[`media`](https://developer.anduril.com/reference/rest/entities/publish-entity#request.body.media) component and append a new `MediaItem`.
This `MediaItem` references `manifest.csv`.
To run the code, replace the following:

- `object_path`: The unique path of the object in Lattice.
For this example, use `/api/v1/objects/manifest.csv`.
- `entity_id`: The ID of the entity you want to associate with the object.

GogRPC/GogRPC/RustJavaPythonTypescript

```
package main

import (
	"context"
	"fmt"
	"net/http"
	"os"
	"time"

	Lattice "github.com/anduril/lattice-sdk-go/v4"
	"github.com/anduril/lattice-sdk-go/v4/client"
	"github.com/anduril/lattice-sdk-go/v4/option"
)

func main() {
	latticeEndpoint := os.Getenv("LATTICE_ENDPOINT")
	clientSecret := os.Getenv("LATTICE_CLIENT_SECRET")
	clientId := os.Getenv("LATTICE_CLIENT_ID")

	// Remove sandboxesToken from the following statements if you are not developing on Sandboxes.
	sandboxesToken := os.Getenv("SANDBOXES_TOKEN")
	if latticeEndpoint == "" || clientId == "" || clientSecret == "" || sandboxesToken == "" {
		fmt.Println("Missing required environment variables")
		os.Exit(1)
	}
	headers := http.Header{}
	headers.Add("Anduril-Sandbox-Authorization", fmt.Sprintf("Bearer %s", sandboxesToken))

	LatticeClient := client.NewClient(
		option.WithClientCredentials(clientId, clientSecret),
		option.WithBaseURL(fmt.Sprintf("https://%s", latticeEndpoint)),
		option.WithHTTPHeader(headers),
	)

	// Set the path to the object and entity ID.
	objectPath := "/api/v1/objects/<objectPath>"
	entityID := "adsbEntity"
	ctx := context.Background()

	// Retrieve the existing entity to get its current media items.
	existingEntity, err := LatticeClient.Entities.GetEntity(ctx, &Lattice.GetEntityRequest{EntityID: entityID})
	if err != nil {
		fmt.Printf("Exception when retrieving entity: %v\n", err)
		return
	}
	var existingMediaItems []*Lattice.MediaItem
	if existingEntity.Media != nil && existingEntity.Media.Media != nil {
		existingMediaItems = existingEntity.Media.Media
	}

	// Create a new media item to append.
	newMediaItem := &Lattice.MediaItem{
		Type:         Lattice.MediaItemTypeMediaTypeImage.Ptr(),
		RelativePath: &objectPath,
	}

	// Append the new media item to the existing ones.
	updatedMediaItems := append(existingMediaItems, newMediaItem)
	updatedMedia := Lattice.Media{
		Media: updatedMediaItems,
	}
	latestTimestamp := time.Now().UTC()

	// Create an entity override with the updated media.
	entityOverride := Lattice.EntityOverride{
		EntityID:  entityID,
		FieldPath: "media.media",
		Entity: &Lattice.Entity{
			EntityID: &entityID,
			Media:    &updatedMedia,
		},
		Provenance: &Lattice.Provenance{
			IntegrationName:  Lattice.String("your_integration_name"),
			DataType:         Lattice.String("test_data"),
			SourceUpdateTime: Lattice.Time(latestTimestamp),
		},
	}

	// Override the entity's media.
	_, err = LatticeClient.Entities.OverrideEntity(
		ctx,
		&entityOverride,
	)

	if err != nil {
		fmt.Printf("Exception: %v\n", err)
	} else {
		fmt.Printf("Successfully appended media to entity: %s\n", entityID)
	}
}
```

[4](https://developer.anduril.com/guides/objects/upload#verify-the-response-2)

### Verify the response

If successful, you’ll see the entity updated with the following `media` component:

```
"media": {
    "media": [\
    {\
        "url": "",\
        "type": "MEDIA_TYPE_IMAGE",\
        "relativePath": "/api/v1/objects/cessna.jpg"\
    },\
    {\
        "url": "",\
        "type": "MEDIA_TYPE_INVALID",\
        "relativePath": "/api/v1/objects/manifest.csv"\
    }\
    ]
}
```

Because the manifest is not an image used as a thumbnail, by default,
Lattice assigns the item’s media type to `MEDIA_TYPE_INVALID`.

## What’s next?

- Learn how to [fetch and process](https://developer.anduril.com/guides/objects/download) objects from Lattice.
- Learn more about the [Object API](https://developer.anduril.com/reference/rest/objects/list-objects) in the _Lattice API Reference_.

![Shows the panel where you can search for the surface vessel.](https://developer.anduril.com/guides/objects/_files/anduril.docs.buildwithfern.com/58c183e84a49edc4eb2b0c8a5db18eb71e36c6ebdfd68091cd2950a16858f5dc/assets/images/screenshots/lattice-ui-track-panel-n113pf.png)

![Shows the uploaded track thumbnail.](https://developer.anduril.com/guides/objects/_files/anduril.docs.buildwithfern.com/809906483da2a3d55750d8faf43dcf0d27019d45e3defc038755686b2a8579a5/assets/images/screenshots/lattice-ui-track-panel-n113pf-thumbnail.png)